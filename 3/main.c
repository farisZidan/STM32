#include <stdint.h>

// Cortex M3
#define SYSTICK_CSR   *(volatile uint32_t *)0xE000E010 // SysTick Control and Status Register
#define SYSTICK_RVR   *(volatile uint32_t *)0xE000E014 // SysTick Reload Value Register
#define SYSTICK_CVR   *(volatile uint32_t *)0xE000E018 // SysTick Current Value Register
#define SYSTICK_CALIB *(volatile uint32_t *)0xE000E01C // SysTick Calibration Value Register

// STM32
#define RCC_BASE    0x40021000
#define GPIOA_BASE  0x40010800
#define GPIOC_BASE  0x40011000
#define USART1_BASE 0x40013800

// I/O PIN Configuration & Data Registers
#define RCC_APB2ENR *(volatile uint32_t *)(RCC_BASE + 0x18)
#define GPIOA_CRL   *(volatile uint32_t *)(GPIOA_BASE + 0x00) // Configuration Register Low
#define GPIOA_CRH   *(volatile uint32_t *)(GPIOA_BASE + 0x04) // Configuration Register High
#define GPIOA_ODR   *(volatile uint32_t *)(GPIOA_BASE + 0x0C) // Output Data Register
#define GPIOC_CRH   *(volatile uint32_t *)(GPIOC_BASE + 0x04) // Configuration Register
#define GPIOC_ODR   *(volatile uint32_t *)(GPIOC_BASE + 0x0C) // Output Data Register

// USART1 Registers
#define USART1_SR   *(volatile uint32_t *)(USART1_BASE + 0x00) // Status Register
#define USART1_DR   *(volatile uint32_t *)(USART1_BASE + 0x04) // Data Register
#define USART1_BRR  *(volatile uint32_t *)(USART1_BASE + 0x08) // Baud Rate Register
#define USART1_CR1  *(volatile uint32_t *)(USART1_BASE + 0x0C) // Control Register 1


// Global tick count variable to keep track of SysTick interrupts
volatile uint32_t tick_count = 0;

void SysTick_Handler(void) {
    tick_count++;
}

void delay(volatile uint32_t count) {
    while(count--) {
        __asm volatile("nop"); // No Operation - prevents the compiler from optimizing away the loop
    }
}

void print_string(const char *str) {
    while (*str != '\0') { // Loop until the null terminator is reached
        // Wait until Transmit Data Register is empty
        while ((USART1_SR & (1 << 7)) == 0) {
            __asm volatile("nop"); 
        }
        USART1_DR = *str++; // Transmit current character and move to the next
    }
}   


int main(void) {

    // PIN A, C, USART1 Enable.
    RCC_APB2ENR |= (1 << 2) | (1 << 4) |(1 << 14);   

    // Read-Modify-Write for mode setting (0010) GPIOC 13, 14, 15 for Blinky LED
    GPIOC_CRH &= ~((0xF << 20) | (0xF << 24) | (0xF << 28));    // 1) Set 1111 (0xf) from bit 20 to 23, 24 to 27, and 28 to 31. 2) Reverse value 1 to 0, and vice versa. 3) Bit 20-23, 24-27, and 28-31 set to zero.
    GPIOC_CRH |= (0b0010 << 20) | (0b0010 << 24) | (0b0010 << 28);

    // Set GPIOA 0 to output mode (0010) for LED
    GPIOA_CRL &= ~((0xF << 0) | (0xF << 4) | (0xF << 8)); // Clear bits for PA0, PA1, and PA2
    GPIOA_CRL |= (0b0010 << 0) | (0b0010 << 4) | (0b0010 << 8); // Set PA0, PA1, and PA2 to output mode (0010)

    // GPIOA 9 for TX (1010), and GPIOA 10 for RX (0100; Even though it's floating input by default, I set it to input mode for clarity)
    GPIOA_CRH &= ~((0xF << 4) | (0xF << 8)); 
    GPIOA_CRH |= (0b1010 << 4) | (0b0100 << 8);

    // USART1 Configuration: 115200 bps (Clock 8 MHz)
    USART1_BRR = 0x0045;    // 8 MHz // 115200 bps = 69 (0x45 in hexadecimal)
    USART1_CR1 |= (1 << 13) | (1 << 3) | (1 << 2); // USART Enable, Transmitter Enable, Receiver Enable


    print_string("Hello, World!\r\n");
    print_string("Aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!\r\n");




    // SysTick Timer Configuration: 1 ms tick (assuming 8 MHz clock)
    SYSTICK_RVR = 7999;
    SYSTICK_CVR = 0; // Clear current value
    SYSTICK_CSR = 0x07; // Enable SysTick, enable interrupt, use processor clock
    
    // Initial LED states
    GPIOA_ODR &= ~((1 << 0) | (1 << 1)); // Turn on LED connected to GPIOA 0 and 1
    GPIOA_ODR |= (1 << 2); // Turn off LED connected to GPIOA 2


    // PC13 Blinky LED State Variables
    uint8_t state_PC13 = 0; // 0: Wake, 1: Quick Sleep, 2: Wake, 3: Deep Sleep
    uint32_t delay_PC13 = 100; // Initial delay for PC13 state changes  

     // Set local variable to store the last tick count for timing
    uint32_t last_tick_PC13 = 0;
    uint32_t last_tick_PA1_PA2 = 0;


    while(1) {

        // Blinky LED on GPIOC 13; 2 fast blinks every 1 second
        if (tick_count - last_tick_PC13 >= delay_PC13) {
            
            if (state_PC13 == 0) {
                // 0: Wake
                GPIOC_ODR &= ~(1 << 13); 
                delay_PC13 = 100;    
                state_PC13 = 1;
                
            } else if (state_PC13 == 1) {
                // 1: Quick Sleep
                GPIOC_ODR |= (1 << 13);  
                delay_PC13 = 100;      
                state_PC13 = 2;
                
            } else if (state_PC13 == 2) {
                // 2: Wake
                GPIOC_ODR &= ~(1 << 13); 
                delay_PC13 = 100;       
                state_PC13 = 3;          
                
            } else if (state_PC13 == 3) {
                // 3: Deep Sleep
                GPIOC_ODR |= (1 << 13);  
                delay_PC13 = 700;        
                state_PC13 = 0;          
            }
            last_tick_PC13 = tick_count; 
        }

        // BLinky LED on GPIOA 1 and 2
        if (tick_count - last_tick_PA1_PA2 >= 1000) { // Toggle every 1000 ms
            GPIOA_ODR ^= (1 << 1) | (1 << 2); // Toggle GPIOA 1 and 2
            last_tick_PA1_PA2 = tick_count; // Update last tick count for PA1 and PA2
        }
        
    }
}