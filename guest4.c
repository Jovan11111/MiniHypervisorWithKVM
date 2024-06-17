#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void 
__attribute__((noreturn)) 
__attribute__((section(".start"))) _start(void) {
    uint16_t port = 0xE9;
    char input[100];
    int i = 0;
    
    // Read data from mini-hypervisor via port 0xE9
    while (1) {
        char value = inb(port);
        if (value == 'x' || i >= sizeof(input) - 1) {
            break;
        }
        input[i++] = value;
        // Echo back each character to indicate reception
    }
    
    // Null-terminate the input string
    input[i] = '\0';
    
    // Print the received data
    for (int j = 0; j < i; j++) {
        outb(0xE9, input[j]);
    }
    outb(0xE9, '\n');
    
    // Enter an infinite loop to halt execution
    for (;;) {
        asm("hlt");
    }
}
