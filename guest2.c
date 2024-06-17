#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static void outb(uint16_t port, uint8_t value) {
	asm("outb %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}

static uint8_t inb(uint16_t port){
	uint8_t ret;
	asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

void
__attribute__((noreturn))
__attribute__((section(".start")))
_start(void) {
    uint16_t port = 0xE9;
	const char *p;

	while(1){
		char value = inb(port);

		if(value == '0') for (p = "Nula je prvi put koriscena u indiji, u 5. veku\n"; *p; ++p) outb(0xE9, *p);
		else if(value == '1') for (p = "Kvadrat broja jedan je jos uvek 1, on je jedini takav pozitivan broj\n"; *p; ++p) outb(0xE9, *p);
		else if(value == '2') for (p = "Broj 2 je jedini paran prost broj\n"; *p; ++p) outb(0xE9, *p);
		else if(value == '3') for (p = "Najmanji potreban broj tacaka za definisanje ravni je 3\n"; *p; ++p) outb(0xE9, *p); 
		else if(value == '4') for (p = "U Kini se smatra da je broj 4 nesrecan jer mu je izgovor slican kao rec smrt\n"; *p; ++p) outb(0xE9, *p); 
		else if(value == '5') for (p = "Broj 5 se pominje u zlatnom preseku\n"; *p; ++p) outb(0xE9, *p); 
		else if(value == '6') for (p = "6 je prvo savrseni broj\n"; *p; ++p) outb(0xE9, *p); 
		else if(value == '7') for (p = "Duga se sastoji iz 7 boja\n"; *p; ++p) outb(0xE9, *p); 
		else if(value == '8') for (p = "Ne padaj tako lako na fore ;)\n"; *p; ++p) outb(0xE9, *p); 
		else if(value == '9') for (p = "Pluton je 9 planeta u Suncevom sistemu\n"; *p; ++p) outb(0xE9, *p);
		else break; 
	}

    for (;;)
        asm("hlt");
}
