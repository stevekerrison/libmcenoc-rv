#include "isr.h"
#include "uart.h"

extern void(*irq_vec_ptr)(void);
extern int *irq_a0;
//extern void(*_exit)(void);

static void (*irq_table[IRQ_TABLE_SIZE])(void) = {
    (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1,
    (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1,
    (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1,
    (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1, (void *)-1,
};

void reset(void) {
    uart_tx_buf("\r\nResetting...\r\n\r\n", 0, UART_BLOCK | UART_DRAIN);
    irq_mask(-1);
    uart_deinit();
    __asm__ __volatile__("j 4"::);
}

void isr_reset(void) {
    irq_setq(q0, (unsigned int)reset);
    irq_ret();
}

int isr_setup(unsigned int irq_num, void (*fn)(void)) {
    if (irq_num >= IRQ_TABLE_SIZE) {
        return 0;
    }
    irq_table[irq_num] = fn;
    return 1;
}

unsigned int isr_mask(unsigned int mask) {
    unsigned imask = 0;
    for (int i = 0; i < IRQ_TABLE_SIZE; i += 1) {
        if (irq_table[i] == (void *)-1) {
            imask |= (1 << i);
        }
    }
    imask |= mask;
    irq_mask(imask);
    return imask;
}

void isr(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3,
    unsigned int a4, unsigned int a5, unsigned int a6, unsigned int a7) {
    unsigned int trig = irq_getq(q1);
    /* Handle real interrupts before software interrupt */
    for (int i = 0; i < IRQ_TABLE_SIZE; i += 1) {
        if (i != 1 && ((trig >> i) & 1) && irq_table[i] != (void *)-1) {
            irq_table[i]();
        }
    }
    /* Now handle ecall if there was one */
    if (((trig >> 1) & 1) && irq_table[1] != (void *)-1) {
        /* Get arguments back from the original stack */
        int result;
        int (*fn)(unsigned int, unsigned int, unsigned int, unsigned int,
            unsigned int, unsigned int, unsigned int, unsigned int) = (void *)irq_table[1];
        result = fn(a0, a1, a2, a3, a4, a5, a6, a7);
        irq_a0 = (int *)result;
    }
    return;
}

void isr_install(void) {
    irq_vec_ptr = (void *)isr;
}
