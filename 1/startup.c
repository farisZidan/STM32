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
    (uint32_t)&_estack,         /* 0x0000: Initial Stack Pointer (MSP) */
    (uint32_t)&Reset_Handler,   /* 0x0004: Reset Vector (Fungsi Start) */
    (uint32_t)&Default_Handler, /* 0x0008: NMI Handler (Error Non-Maskable) */
    /* ... Masih banyak lagi sebenarnya, tapi untuk Blinky cukup ini ... */
};