#define _GNU_SOURCE // omogucava GNU prosirenja
#include <stdio.h> // IO
#include <stdlib.h> // std C funkcije
#include <unistd.h> // citanje i pisanje u fajlove
#include <fcntl.h> // jos manipulisanja fajlovima
#include <errno.h> // makroi za greske
#include <sys/ioctl.h> // kontrola uredjaja
#include <sys/mman.h> // upravljanje memorijom
#include <string.h> // funkcije za stringove
#include <stdint.h> // tipovi po bitovima
#include <linux/kvm.h> // KVP API
#include <pthread.h> // POSIX niti

// Oznaka da je ulaz prisutan u tabeli stranica
#define PDE64_PRESENT 1
// Oznaka da ulaz moze da se korisni za citanje i pisanje
#define PDE64_RW (1U << 1)
// Oznaka da ulaz ima korisnicki pristup
#define PDE64_USER (1U << 2)
// Oznaka da ulaz koristi velicinu stranice
#define PDE64_PS (1U << 7)

// Oznaka za postavljanje prosirenja fizicke adrese u CR4
#define CR4_PAE (1U << 5)

// Protected Mode Enable u CR0
#define CR0_PE 1u
// Paging u CR0
#define CR0_PG (1U << 31)

// Long mode enabled and active
#define EFER_LME (1U << 8)
#define EFER_LMA (1U << 10)

int TEST1 = 0;
int TEST2 = 0;


// Struktura koja cuva fajl deskriptore, virtuelnu masinu i virtuelni CPU,
// pokazivace na memorjju
struct vm {
	int kvm_fd; // deskriptor za KVM uredjaj
	int vm_fd; // deskriptor za VM
	int vcpu_fd; // deskriptor za VCPU
	char *mem; // pokazivac za dodeljenu memoriju za VM
	struct kvm_run *kvm_run; // Pokazivac na runtime strukturu
};

struct thread_params{
	struct vm* vm;
	const char* guest;
	size_t mem_size;
	size_t page_size;
};

// Funkcija koja sluzi za inicijalizaciju virtuelne masine koja otvori KVM uredjaj
// kreira VM, dodeli joj region, i mapira memoriju u runtime strukturu
int init_vm(struct vm *vm, size_t mem_size)
{
	// Struktura koja definise region koji ce VM koristiti
	struct kvm_userspace_memory_region region;
	// Velicina mem mape za runtime strukturu
	int kvm_run_mmap_size;

	// Ovo otvara kvm uredjaj
	vm->kvm_fd = open("/dev/kvm", O_RDWR);

	// Proverava da li je uredjaj otvoren, ako jeste ide dalje, ako nije ispisuje gresku vraca -1
	if (vm->kvm_fd < 0) {
		perror("KVM not oppened correclty...\n");
		return -1;
	}

	// Ovako se kreira virtuelna masina
	vm->vm_fd = ioctl(vm->kvm_fd, KVM_CREATE_VM, 0); 
	// Provera da li je VM kreirana, ako nije gasi program ispisuje gresku
	if (vm->vm_fd < 0) {
		perror("Virtual macine not oppened correctly...\n");
		return -1;
	}

	// Kreira se memorijska mapa koju ce koristiti VM, daju joj se prava pristupa
	vm->mem = mmap(NULL, mem_size, PROT_READ | PROT_WRITE,
		   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	
	// Proverava da li je mem mapa pravilno dodeljena VM. Ako nije prekida
	if (vm->mem == MAP_FAILED) {
		perror("Failed to allocate the memory map for VM...\n");
		return -1;
	}

	region.slot = 0; // slot od regiona
	region.flags = 0; // nije potrebno postaviti nikakve flegove
	region.guest_phys_addr = 0; // fizicka adresa regiona u gostu
	region.memory_size = mem_size; // velicina regiona u memoriji
	// Postavlja adresu regiona u korisnickom prostoru
	region.userspace_addr = (unsigned long)vm->mem; 
	// Postavlja region u VM, ako nije uspesno postavljen, ispisuje gresku i gasi
    if (ioctl(vm->vm_fd, KVM_SET_USER_MEMORY_REGION, &region) < 0) {
		perror("Failed to set user memory region...\n");
        return -1;
	}

	// Kreira se virtuelni CPU
	vm->vcpu_fd = ioctl(vm->vm_fd, KVM_CREATE_VCPU, 0);
    // Provera da li je VCPU kreiran, ako nije ispisuje se greska
	if (vm->vcpu_fd < 0) {
		perror("Failed to create virtual CPU...\n");
        return -1;
	}

	// runtime struktura dbija potrebnu velicinu memorije
	kvm_run_mmap_size = ioctl(vm->kvm_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
    // Ako strukturi nije dodeljena memorija pravilno, ispisuje se greska
	if (kvm_run_mmap_size <= 0) {
		perror("Failed to get memory size to CPU...\n");
		return -1;
	}

	// Mapira se memorija u runtime strukturi, dodeljuju se prava pristupa
	vm->kvm_run = mmap(NULL, kvm_run_mmap_size, PROT_READ | PROT_WRITE,
			     MAP_SHARED, vm->vcpu_fd, 0);
	// Ako to nije uspesno odradjeno, ispisuje se greska
	if (vm->kvm_run == MAP_FAILED) {
		perror("Failed to map the memory in runtime structure...\n");
		return -1;
	}

	return 0;
}

// Staticka funkcija koja postavlja 64-bitni segment
static void setup_64bit_code_segment(struct kvm_sregs *sregs)
{	
	// Ovo je zapravo strukura segmenta
	struct kvm_segment seg = {
		.base = 0,
		.limit = 0xffffffff,
		.present = 1, // Govori da li je segmetn ucitan u memorjiu
		.type = 11, // da li moze da se radi execute, read, accessed
		.dpl = 0, // Descriptor Privilage Level: 0 (0, 1, 2, 3)
		.db = 0, // Default size - ima vrednost 0 u long modu
		.s = 1, // Code/data tip segmenta
		.l = 1, // Long mode - 1
		.g = 1, // 4KB granularnost
	};

	// Postavlja se kod segment CS registra na segment strukturu
	sregs->cs = seg;

	seg.type = 3; // Data: read, write, accessed
	// Svi drugi segmenti se postavljaju na ovaj segment
	sregs->ds = sregs->es = sregs->fs = sregs->gs = sregs->ss = seg;
}



static void setup_paging(struct vm *vm, struct kvm_sregs *sregs, int page_size, int num_pages)
{
    uint64_t page = 0;
    uint64_t pml4_addr = 0x1000;
    uint64_t *pml4 = (void *)(vm->mem + pml4_addr);

    uint64_t pdpt_addr = 0x2000;
    uint64_t *pdpt = (void *)(vm->mem + pdpt_addr);

    uint64_t pd_addr = 0x3000;
    uint64_t *pd = (void *)(vm->mem + pd_addr);

    if (page_size == 0x200000) {  // ako je stranica 2MB
        uint64_t pt_addr = 0;
        uint64_t *pt = (void *)(vm->mem + pt_addr);

		pml4[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pdpt_addr;
        pdpt[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pd_addr;
        pd[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | PDE64_PS | page;
		for(int i = 0; i < num_pages; i++){
			pt[i] = page | PDE64_PRESENT | PDE64_RW | PDE64_USER;
			page += 0x200000;
		}

	} else if (page_size == 0x1000) {  // ako je stranica 4KB
        uint64_t pt_addr = 0x4000;
        uint64_t *pt = (void *)(vm->mem + pt_addr);
        pml4[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pdpt_addr;
        pdpt[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pd_addr;
        pd[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pt_addr;

		for(int i = 0; i < num_pages; i++) {
			pt[i] = page | PDE64_PRESENT | PDE64_RW | PDE64_USER;
	 		page += 0x1000;
		}
    }

	// Registar koji ukazuje na PML4 tabelu stranica. Odavde kreće mapiranje VA u PA.
    sregs->cr3 = pml4_addr;
    sregs->cr4 = CR4_PAE; // "Physical Address Extension" mora biti 1 za long mode.
    sregs->cr0 = CR0_PE | CR0_PG; // Postavljanje "Protected Mode" i "Paging"
    sregs->efer = EFER_LME | EFER_LMA; // Postavljanje  "Long Mode Active" i "Long Mode Enable"

	// Inicijalizacija segmenata procesora.
    setup_64bit_code_segment(sregs);
}

void *run_thread(void *arg){
	struct thread_params* args = (struct thread_params*)arg;
	struct vm vm = *args->vm;
	const char* guest = args->guest;
	size_t mem_size = args->mem_size;
	size_t page_size = args->page_size;
	int num_pages = 0;
	FILE* img;
	int stop = 0;
	int ret = 0;

	struct kvm_sregs sregs;
	struct kvm_regs regs;

	if (init_vm(&vm, mem_size)) {
		printf("Failed to initialize the VM\n");
		pthread_exit(NULL);
	}

	if (ioctl(vm.vcpu_fd, KVM_GET_SREGS, &sregs) < 0) {
		perror("KVM_GET_SREGS");
		pthread_exit(NULL);
	}
	
	num_pages = mem_size / page_size;

	setup_paging(&vm, &sregs, page_size, num_pages);

    if (ioctl(vm.vcpu_fd, KVM_SET_SREGS, &sregs) < 0) {
		perror("KVM_SET_SREGS");
		pthread_exit(NULL);
	}

	memset(&regs, 0, sizeof(regs));
	regs.rflags = 2;
	regs.rip = 0;
	
	if(mem_size == 0x200000) regs.rsp = 2 << 20;
	else if(mem_size == 0x400000) regs.rsp = 4 << 20;
	else regs.rsp = 8 << 20;

	regs.rsp = 2 << 20;

	
	if (ioctl(vm.vcpu_fd, KVM_SET_REGS, &regs) < 0) {
		perror("KVM_SET_REGS");
		pthread_exit(NULL);
	}
	
	img = fopen(guest, "r");
	if (img == NULL) {
		printf("Can't open binary file\n");
		pthread_exit(NULL);
	}

	char *p = vm.mem;
  	while(feof(img) == 0) {
    	int r = fread(p, 1, 1024, img);
    	p += r;
  	}
  	fclose(img);

	while(stop == 0) {
		ret = ioctl(vm.vcpu_fd, KVM_RUN, 0);
		if (ret == -1) {
			printf("KVM_RUN failed\n");
			pthread_exit(NULL);
		}

		switch (vm.kvm_run->exit_reason) {
			case KVM_EXIT_IO:

				if (vm.kvm_run->io.direction == KVM_EXIT_IO_IN && vm.kvm_run->io.port == 0xE9) {
					if(TEST1 == 1){
						char input[100];
						char visak;
						scanf("%s", input);
						for(int i = 0; i < strlen(input); i++){
							*((char *)vm.kvm_run + vm.kvm_run->io.data_offset) = input[i];
						}
					}
					if(TEST2 == 1){
						static int input_index = 0;
						static char input[100];
						if (input_index == 0) {
							// Read full input string initially
							scanf("%s", input);
						}
						// Send one character at a time to the guest
						if (input[input_index] != '\0') {
							*((char *)vm.kvm_run + vm.kvm_run->io.data_offset) = input[input_index++];
						} else {
							input_index = 0;  // Reset index for next input
						}
					}
				} else if (vm.kvm_run->io.direction == KVM_EXIT_IO_OUT && vm.kvm_run->io.port == 0xE9) {
					char *p = (char *)vm.kvm_run;
					printf("%c", *(p + vm.kvm_run->io.data_offset));
				}
				continue;
			case KVM_EXIT_HLT:
				printf("KVM_EXIT_HLT\n");
				stop = 1;
				break;
			case KVM_EXIT_INTERNAL_ERROR:
				printf("Internal error: suberror = 0x%x\n", vm.kvm_run->internal.suberror);
				stop = 1;
				break;
			case KVM_EXIT_SHUTDOWN:
				printf("Shutdown\n");
				stop = 1;
				break;
			default:
				printf("Exit reason: %d\n", vm.kvm_run->exit_reason);
				break;
    	}
  	}
}

int main(int argc, char *argv[])
{
	size_t mem_size = 0;
	size_t page_size = 0;
	int num_pages = 0;
	int num_guests = 0;

	
	if (argc < 7) {
    	perror("The program requests a command in a format: ./mini_hypervisor [--memory/-m] <size> [--page, -p] <size> [--guest/-g] <guest1.img> <guest2.img>...\n");
    	return 1;
  	}

	if(strcmp(argv[1], "-m") != 0 && strcmp(argv[1], "--memory") != 0){
		perror("The program requests a command in a format: ./mini_hypervisor [--memory/-m] <size> [--page, -p] <size> [--guest/-g] <guest1.img> <guest2.img>...\n");
    	return 1;
	}

	if(strcmp(argv[3], "-p") != 0 && strcmp(argv[3], "--page") != 0){
		perror("The program requests a command in a format: ./mini_hypervisor [--memory/-m] <size> [--page, -p] <size> [--guest/-g] <guest1.img> <guest2.img>...\n");
    	return 1;
	}

	if(strcmp(argv[5], "-g") != 0 && strcmp(argv[5], "--guest") != 0){
		perror("The program requests a command in a format: ./mini_hypervisor [--memory/-m] <size> [--page, -p] <size> [--guest/-g] <guest1.img> <guest2.img>...\n");
    	return 1;
	}
	if(argc == 7) TEST2 = 1;
	if(argc == 9) TEST1 = 1;

	if(strcmp(argv[2], "2") == 0){
		mem_size = 0x200000;
	} else if(strcmp(argv[2], "4") == 0){
		mem_size = 0x400000;
	}else if(strcmp(argv[2], "8") == 0){
		mem_size = 0x800000;
	} else{
		perror("Memory argument should be 2, 4, or 8, for physical guest memory of 2MB, 4MB or 8MB\n");
		return 1;
	}

	printf("\nMem size is: %ld bytes\n", mem_size);

	if(strcmp(argv[4], "2") == 0){
		page_size = 0x200000;
		printf("\nPage size is 2MB\n");
	} else if(strcmp(argv[4], "4") == 0){
		page_size = 0x1000;
		printf("\nPage size is 4KB\n");
	} else{
		perror("Page argument should be 2 o 4, for page size of 2MB or 4KB\n");
		return 1;
	}

	// Broj gostiju govori koliko je potrebno napraviti niti 
	num_guests = argc-6;

	struct vm* virtual_machines = calloc(num_guests, sizeof(struct vm));
	pthread_t* threads = calloc(num_guests, sizeof(pthread_t));
	struct thread_params* args = calloc(num_guests, sizeof(struct thread_params));

	for(int i = 0; i < num_guests; i++){
		args[i].guest = argv[6+i];
		args[i].mem_size = mem_size;
		args[i].page_size = page_size;
		args[i].vm = &virtual_machines[i];
	}

	for(int i = 0; i < num_guests; i++){
		pthread_create(&threads[i], NULL, run_thread, &args[i]);
	}

	for(int i = 0; i < num_guests; i++){
		pthread_join(threads[i], NULL);
	}

	free(virtual_machines);
	free(threads);
	free(args);	
}
