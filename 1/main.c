#include <stdint.h>

#define RCC_BASE    0x40021000
#define GPIOC_BASE  0x40011000

#define RCC_APB2ENR *(volatile uint32_t *)(RCC_BASE + 0x18)
#define GPIOC_CRH   *(volatile uint32_t *)(GPIOC_BASE + 0x04)
#define GPIOC_ODR   *(volatile uint32_t *)(GPIOC_BASE + 0x0C)


void delay(volatile uint32_t count) {
    while(count--) {
        __asm volatile("nop"); // No Operation - prevents the compiler from optimizing away the loop
    }
}

int main(void) {
    // PIN C 13 Enable
    RCC_APB2ENR |= (1 << 4);

    // Read-Modify-Write
    GPIOC_CRH &= ~(0xF << 20);  // 1) Set 1111 (0xf) from bit 20 to 23. 2) Reverse value 1 to 0, and vice versa. 3) Bit 20-23 set to zero.  
    GPIOC_CRH |= (0b0010 << 20);     // Fill bit 20-23.

    while(1){
        GPIOC_ODR |= (1 << 13);

        delay(1000000);

        GPIOC_ODR &= ~(1 << 13);

        delay(500000);
    }
}