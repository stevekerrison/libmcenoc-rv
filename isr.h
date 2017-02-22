#ifndef ISR_H
#define ISR_H

#include "irq-ops.h"

#define IRQ_TABLE_SIZE 32

/* Apply given mask to IRQs, guarded by IRQs that have an ISR setup */
unsigned int isr_mask(unsigned int mask);

void reset(void);
void isr_reset(void);
void isr_install(void);
int isr_setup(unsigned int irq_num, void (*fn)(void));

#endif //ISR_H
