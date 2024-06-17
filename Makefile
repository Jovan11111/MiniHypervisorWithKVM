all: guest1.img guest2.img guest3.img guest4.img mini_hypervisor

gost1: guest1.img mini_hypervisor

gost2: guest1.img mini_hypervisor

gost3: guest1.img mini_hypervisor

gost4: guest4.img mini_hypervisor

mini_hypervisor: mini_hypervisor.c
	gcc mini_hypervisor.c -pthread -o mini_hypervisor

guest1.img: guest1.o
	ld -T guest.ld guest1.o -o guest1.img

guest1.o: guest1.c
	$(CC) -m64 -ffreestanding -fno-pic -c -o $@ $^

guest2.img: guest2.o
	ld -T guest.ld guest2.o -o guest2.img

guest2.o: guest2.c
	$(CC) -m64 -ffreestanding -fno-pic -c -o $@ $^

guest3.img: guest3.o
	ld -T guest.ld guest3.o -o guest3.img

guest3.o: guest3.c
	$(CC) -m64 -ffreestanding -fno-pic -c -o $@ $^

guest4.img: guest4.o
	ld -T guest.ld guest4.o -o guest4.img

guest4.o: guest4.c
	$(CC) -m64 -ffreestanding -fno-pic -c -o $@ $^

clean:
	rm -f mini_hypervisor guest1.o guest1.img guest2.o guest2.img guest3.o guest3.img guest4.img guest4.o