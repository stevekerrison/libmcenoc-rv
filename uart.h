#ifndef UART_H
#define UART_H

#include <stdlib.h>

#define UART_BLOCK      0
#define UART_NONBLOCK   1
#define UART_DRAIN      2

#define OPCODE_URC 16
#define OPCODE_UWC 17
#define OPCODE_URX 18
#define OPCODE_UTX 19

#define TX_ACTIVE   0x1
#define RX_RECEIVED 0x2
#define TX_IRQ_EN   0x4
#define RX_IRQ_EN   0x8

#define BUFSIZE 128 /* Must be 2**n */
#define BUFMASK (BUFSIZE-1)

unsigned int inline uart_urc(void) {
    unsigned int status;
    __asm__ __volatile__("custom0 %0,0,0,%1":"=r"(status):"i"(OPCODE_URC));
    return status;
}

void inline uart_uwc(unsigned int ctrl) {
    __asm__ __volatile__("custom0 zero,%0,0,%1"::"r"(ctrl),"i"(OPCODE_UWC));
}

void inline uart_utx(char c) {
    __asm__ __volatile__("custom0 zero,%0,0,%1"::"r"(c),"i"(OPCODE_UTX));
}

char inline uart_urx(void) {
    char c;
    __asm__ __volatile__("custom0 %0,0,0,%1":"=r"(c):"i"(OPCODE_URX));
    return c;
}

int uart_tx_drain();

void uart_init(void);

void uart_deinit(void);

void uart_irq(void);

int uart_rx_buf(char *buf, size_t len, int flags);

int uart_rx_char(char *c, int flags);

int uart_tx_char(char c, int flags);

int uart_tx_buf(char * buf, size_t len, int flags);


#endif //UART_H
