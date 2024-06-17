#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

static uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void
__attribute__((noreturn))
__attribute__((section(".start")))
_start(void) {
    uint16_t port = 0xE9;
    const char *p;
    char value;

    while (1) {
        value = inb(port);

        if (value == '0') p = "Nula je jedini broj kojim se ne moze deliti\n";
        else if (value == '1') p = "Jedan je jedini broj koji je deljiv samo jednim brojem\n";
        else if (value == '2') p = "Da ne postoji broj 2, ne bi postojali kompjuteri\n";
        else if (value == '3') p = "Broj je deljiv sa 3 ako mu je zbir cifara deljiv sa 3\n";
        else if (value == '4') p = "Broj je deljiv sa 4 ako su mu poslednje dve cifre deljive sa 4\n";
        else if (value == '5') p = "Svi stepeni 5ice se zavrsavaju brojem 5\n";
        else if (value == '6') p = "NA ruletu ima 6 na kvadrat brojeva, njihov zbir je 666\n";
        else if (value == '7') p = "Decimalni ostatak pri deljenju sa 7 je shiftovana vrednost ostatka pri deljenju susdenog broja sa 7 \n";
        else if (value == '8') p = "Ne padaj tako lako na fore ;)\n";
        else if (value == '9') p = "8 i jedan pamtim ;))\n";
        else break;

        // Send the message
        for (; *p; ++p) {
            outb(port, *p);
        }
    }

    // Enter an infinite loop to halt execution
    for (;;) {
        asm("hlt");
    }
}
