#ifndef IRQ_OPS_H
#define IRQ_OPS_H

#define IRQ_UART        0x00000008
#define IRQ_MCENOC_SW   0x00000010

#define OPCODE_GETQ     0
#define OPCODE_SETQ     1
#define OPCODE_RETIRQ   2
#define OPCODE_MASKIRQ  3
#define OPCODE_WAITIRQ  4
#define OPCODE_TIMER    5

#define q0 0
#define q1 1
#define q2 2
#define q3 3

unsigned int inline irq_getq(unsigned int q) {
    unsigned int ret;
    __asm__ __volatile__("custom0 %0,%1,0,%2":"=r"(ret):"i"(q),"i"(OPCODE_GETQ));
    return ret;
}

void inline irq_setq(unsigned int q, unsigned int s) {
    __asm__ __volatile__("custom0 %0,%1,0,%2"::"i"(q),"r"(s),"i"(OPCODE_SETQ));
}

void inline irq_ret() {
    __asm__ __volatile__("custom0 0,0,0,%0"::"i"(OPCODE_RETIRQ));
}

inline unsigned int irq_mask(unsigned int mask) {
    unsigned int orig;
    __asm__ __volatile__("custom0 %0,%1,0,%2":"=r"(orig):"r"(mask),"i"(OPCODE_MASKIRQ));
    return orig;
}

unsigned int inline irq_wait() {
    unsigned int pending;
    __asm__ __volatile__("custom0 %0,0,0,%1":"=r"(pending):"i"(OPCODE_WAITIRQ));
    return pending;
}

unsigned int inline irq_timer(unsigned int tv) {
    unsigned int orig;
    __asm__ __volatile__("custom0 %0,%1,0,%2":"=r"(orig):"r"(tv),"i"(OPCODE_TIMER));
    return orig;
}

#endif //IRQ_OPS_H
