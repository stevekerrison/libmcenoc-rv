#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "uart.h"
#include "isr.h"
#include "irq-ops.h"
#include "syscall.h"
#ifdef SYSCALL_DEBUG
    #include <stdio.h>
#endif

static char syscallstr[50] = "SYSCALL\r\n\0";
static char *sbuf = NULL;
static size_t slen = 0;

int uart_tx_buf_thunk(void) {
    uart_tx_buf(sbuf, slen, UART_BLOCK);
    return slen;
}

int uart_rx_thunk(void) {
    uart_rx_char(sbuf, UART_BLOCK);
    return 1;
}

int mcenoc_rv_write(int fd, const char *buf, size_t count) {
    sbuf = (char *)buf;
    slen = count;
    irq_setq(q0, (unsigned int)uart_tx_buf_thunk);
    return count;
}

/* TODO: Return more than one byte at a time */
int mcenoc_rv_read(int fd, char *buf, size_t count) {
    sbuf = buf;
    slen = count;
    irq_setq(q0, (unsigned int)uart_rx_thunk);
    return 1;
}

int mcenoc_rv_fstat(int fd, struct stat *buf) {
    memset(buf, 0, sizeof(struct stat));
    return -1;
}

int mcenoc_rv_brk(void *addr) {
    /* Sure, have as much memory as you want */
    return (int)addr;
}

int mcenoc_rv_syscall(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3,
    unsigned int a4, unsigned int a5, unsigned int a6, unsigned int a7) {
    switch (a7) {
        case SYS_exit:
            isr_reset(); //Never returns
        case SYS_close:
            return 0; //Sure, we'll close those files *wink wink*
        case SYS_read:
            return mcenoc_rv_read(a0, (char *)a1, a2);
        case SYS_write:
            return mcenoc_rv_write(a0, (const char *)a1, a2);
        case SYS_brk:
            return mcenoc_rv_brk((void *)a0);
        case SYS_fstat:
            return mcenoc_rv_fstat((int)a0, (struct stat *)a1);
        default:
#ifdef SYSCALL_DEBUG
            /* Provide useful syscall data, as opposed to just "SYSCALL" */
            sprintf(syscallstr, "S: %08x %08x %08x %08x %08x\r\n", a0, a1, a2, a3, a7);
#endif
            /* Save the old return address and sneak in a new one */
            sbuf = syscallstr;
            slen = 0;
            irq_setq(q0, (unsigned int)uart_tx_buf_thunk);
    }
    return -1;
}
