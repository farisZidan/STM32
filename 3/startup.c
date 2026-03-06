#include <stdint.h>

/* ===============================================================
   1. IMPORT SIMBOL DARI LINKER SCRIPT
   Kita butuh alamat-alamat batas yang sudah kita definisikan di linker.ld
   =============================================================== */
extern uint32_t _estack;    /* Ujung atas RAM (Stack Pointer) */
extern uint32_t _etext;     /* Akhir dari kode di Flash */
extern uint32_t _sdata;     /* Awal data di RAM */
extern uint32_t _edata;     /* Akhir data di RAM */
extern uint32_t _sbss;      /* Awal BSS di RAM */
extern uint32_t _ebss;      /* Akhir BSS di RAM */
extern uint32_t _sidata;    /* Alamat ASAL data di Flash (LMA) */

extern void SysTick_Handler(void);


/* Prototype fungsi main() agar bisa dipanggil */
int main(void);

/* ===============================================================
   2. FUNGSI RESET HANDLER
   Ini adalah kode PERTAMA yang dijalankan CPU saat bangun tidur.
   =============================================================== */
void Reset_Handler(void) {
    /* TAHAP 1: Copy Data dari Flash ke RAM */
    /* Kita butuh 3 pointer: */
    uint32_t *pSrc = &_sidata;  // Sumber (di Flash)
    uint32_t *pDest = &_sdata;  // Tujuan (di RAM)

    /* Selama alamat tujuan belum sampai di ujung (_edata)... */
    while (pDest < &_edata) {
        *pDest = *pSrc; /* Salin isinya */
        pDest++;        /* Geser pointer tujuan maju 1 langkah */
        pSrc++;         /* Geser pointer sumber maju 1 langkah */
    }

    /* TAHAP 2: Bersihkan BSS (Isi dengan Nol) */
    pDest = &_sbss; /* Mulai dari awal BSS */
    while (pDest < &_ebss) {
        *pDest = 0; /* Tulis angka 0 */
        pDest++;    /* Geser maju */
    }

    /* TAHAP 3: Panggil Main Program */
    main();

    /* TAHAP 4: Trap (Jebakan) */
    /* Kalau main() selesai (return), CPU tidak boleh lari ke mana-mana.
       Kita tangkap di infinite loop. */
    while (1);
}

/* ===============================================================
   3. DEFAULT HANDLER
   Kalau ada error/interupsi yang tidak kita handle, lari ke sini.
   =============================================================== */
void Default_Handler(void) {
    while (1);
}

void Button_Interupt(void) {
    /* Tangani interupsi tombol di sini */
    while (1);
}

/* ===============================================================
   4. VECTOR TABLE (PETA INTERUPSI)
   Ini bagian yang WAJIB ada di alamat 0x00000000 (atau 0x08000000).
   CPU Cortex-M3 membaca array ini secara otomatis.
   =============================================================== */
// Atribut ini memaksa array ditaruh di seksi .isr_vector di Linker
uint32_t vectors[] __attribute__((section(".isr_vector"))) = {
    (uint32_t)&_estack,         /* 0 - 0x0000: Initial Stack Pointer */
    (uint32_t)&Reset_Handler,   /* 1 - 0x0004: Reset Vector */
    (uint32_t)&Default_Handler, /* 2 - 0x0008: NMI Handler */
    (uint32_t)&Default_Handler, /* 3 - 0x000C: Hard Fault Handler */
    (uint32_t)&Default_Handler, /* 4 - 0x0010: Memory Management Fault */
    (uint32_t)&Default_Handler, /* 5 - 0x0014: Bus Fault Handler */
    (uint32_t)&Default_Handler, /* 6 - 0x0018: Usage Fault Handler */
    0,                          /* 7 - 0x001C: Reserved */
    0,                          /* 8 - 0x0020: Reserved */
    0,                          /* 9 - 0x0024: Reserved */
    0,                          /* 10- 0x0028: Reserved */
    (uint32_t)&Default_Handler, /* 11- 0x002C: SVCall Handler */
    (uint32_t)&Default_Handler, /* 12- 0x0030: Debug Monitor Handler */
    0,                          /* 13- 0x0034: Reserved */
    (uint32_t)&Default_Handler, /* 14- 0x0038: PendSV Handler */
    (uint32_t)&SysTick_Handler, /* 15- 0x003C: SysTick Handler */
};