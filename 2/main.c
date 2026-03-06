#include <stdint.h>

#define RCC_BASE    0x40021000
#define GPIOA_BASE  0x40010800
#define GPIOC_BASE  0x40011000
#define USART1_BASE 0x40013800

// I/O PIN Configuration & Data Registers
#define RCC_APB2ENR *(volatile uint32_t *)(RCC_BASE + 0x18)
#define GPIOA_CRH   *(volatile uint32_t *)(GPIOA_BASE + 0x04) // Configuration Register
#define GPIOA_ODR   *(volatile uint32_t *)(GPIOA_BASE + 0x0C) // Output Data Register
#define GPIOC_CRH   *(volatile uint32_t *)(GPIOC_BASE + 0x04) // Configuration Register
#define GPIOC_ODR   *(volatile uint32_t *)(GPIOC_BASE + 0x0C) // Output Data Register

// USART1 Registers
#define USART1_SR   *(volatile uint32_t *)(USART1_BASE + 0x00) // Status Register
#define USART1_DR   *(volatile uint32_t *)(USART1_BASE + 0x04) // Data Register
#define USART1_BRR  *(volatile uint32_t *)(USART1_BASE + 0x08) // Baud Rate Register
#define USART1_CR1  *(volatile uint32_t *)(USART1_BASE + 0x0C) // Control Register 1

void delay(volatile uint32_t count) {
    while(count--) {
        __asm volatile("nop"); // No Operation - prevents the compiler from optimizing away the loop
    }
}



int main(void) {
    // PIN A, C, USART1 Enable
    RCC_APB2ENR |= (1 << 2) | (1 << 4) |(1 << 14);

    // Read-Modify-Write for mode setting (0010) GPIOC 13
    GPIOC_CRH &= ~(0xF << 20);    // 1) Set 1111 (0xf) from bit 20 to 23. 2) Reverse value 1 to 0, and vice versa. 3) Bit 20-23 zset to zero.  
    GPIOC_CRH |= (0b0010 << 20);  // Fill bit 20-23.

    // GPIOA 9 for TX (1010), GPIOA 10 for RX (0100; Even though it's floating input by default, I set it to input mode for clarity)
    GPIOA_CRH &= ~(0xF << 4); 
    GPIOA_CRH |= (0b1010 << 4);
    GPIOA_CRH &= ~(0xF << 8); 
    GPIOA_CRH |= (0b0100 << 8);

    // USART1 Configuration: 115200 bps (Clock 8 MHz)
    USART1_BRR = 0x0045;    // 8 MHz // 115200 bps = 69 (0x45 in hexadecimal)
    USART1_CR1 |= (1 << 13) | (1 << 3) | (1 << 2); // USART Enable, Transmitter Enable, Receiver Enable



    while(1){

        // Wait until Transmit Data Register is empty
        while ((USART1_SR & (1 << 7)) == 0) {
            __asm volatile("nop"); 
        }

        USART1_DR = 'W'; // Transmit 'H'    


        GPIOC_ODR |= (1 << 13);
        delay(10000);
        GPIOC_ODR &= ~(1 << 13);
        delay(10000);
        GPIOC_ODR |= (1 << 13);
        delay(100000);
    }
}