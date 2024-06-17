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

		if(value == '0') for (p = "Voda se pretvara u led na 0 stepeni\n"; *p; ++p) outb(0xE9, *p);
		else if(value == '1') for (p = "Broj 1 nije ni prost ni slozen\n"; *p; ++p) outb(0xE9, *p);
		else if(value == '2') for (p = "Najbolja igrica, 2048, je zasnovana na stepenima dvojke\n"; *p; ++p) outb(0xE9, *p);
		else if(value == '3') for (p = "Broj 3 se cesto koristi u bajkama kao magican broj\n"; *p; ++p) outb(0xE9, *p); 
		else if(value == '4') for (p = "Geneticki kod se sastoji iz 4 nukleida, ACGT\n"; *p; ++p) outb(0xE9, *p); 
		else if(value == '5') for (p = "Ljudi imaju 5 cula\n"; *p; ++p) outb(0xE9, *p); 
		else if(value == '6') for (p = "Ceije u sacu od pcela su u obliku sestouglova\n"; *p; ++p) outb(0xE9, *p); 
		else if(value == '7') for (p = "Najcesci broj za koji ljudi kazu da im je srecan je 7\n"; *p; ++p) outb(0xE9, *p); 
		else if(value == '8') for (p = "Ne padaj tako lako na fore ;)\n"; *p; ++p) outb(0xE9, *p); 
		else if(value == '9') for (p = "Na Jupiteru dan traje 9 sati, najkrace u Suncevom sistmeu\n"; *p; ++p) outb(0xE9, *p);
		else break; 
	}

    for (;;)
        asm("hlt");
}
