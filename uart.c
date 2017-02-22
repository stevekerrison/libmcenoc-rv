#include <stdlib.h>
#include "irq-ops.h"
#include "uart.h"

static volatile char txbuf[BUFSIZE] = {};
static volatile size_t txbuf_rpos = 0, txbuf_wpos = 0, txbuf_used = 0;

static volatile char rxbuf[BUFSIZE] = {};
static volatile size_t rxbuf_rpos = 0, rxbuf_wpos = 0, rxbuf_used = 0;

void inline static uart_tx_irq(unsigned int status) {
    /*
     * I guess it's possible we get an interrupt even if we aren't waiting to
     * send, if something misbehaves.
     */
    if (txbuf_used) {
        uart_utx(txbuf[txbuf_rpos++]);
        txbuf_rpos &= BUFMASK;
        txbuf_used--;
    }
    /* Nothing to transmit any more, so disable IRQ on TX ready */
    if (!txbuf_used) {
        uart_uwc(status ^ TX_IRQ_EN);
    }
}

void inline static uart_rx_irq(unsigned int status) {
    rxbuf[rxbuf_wpos++] = uart_urx();
    rxbuf_wpos &= BUFMASK;
    rxbuf_used++;
    if (rxbuf_used == BUFSIZE) {
        /* Buffer is full, so disable IRQ. Any new data will overflow and
         * be lost :( */
        uart_uwc(status ^ RX_IRQ_EN);
    }
}

void uart_init(void) {
    uart_uwc(RX_IRQ_EN);
}

void uart_deinit(void) {
    uart_uwc(0);
}

void uart_irq(void) {
    unsigned int status = uart_urc();
    if ((status & TX_IRQ_EN) && !(status & TX_ACTIVE)) {
        uart_tx_irq(status);
    }
    
    if ((status & RX_IRQ_EN) && (status & RX_RECEIVED)) {
        uart_rx_irq(status);
    }
}

/* Wait for the UART to finish sending everything that's currently buffered */;
int uart_tx_drain(void) {
    int ret;
    unsigned imask = irq_mask(-1);
    irq_mask(imask | IRQ_UART);
    ret = txbuf_used;
    if (txbuf_used > 0) {
        uart_uwc(uart_urc() | TX_IRQ_EN); //Make sure TX IRQ is enabled
    }
    /* Nasty spinlock that allows us to check for txbuf_used reaching 0 */
    while (txbuf_used > 0) {
        /* Wait for the UART interrupt */
        while ((irq_wait() & IRQ_UART) == 0);
        /* Let the interrupt be handled, then mask it off again */
        irq_mask(imask);
        imask = irq_mask(-1);
        irq_mask(imask | IRQ_UART);
    }
    irq_mask(imask);
    return ret;
}

int uart_rx_buf(char * buf, size_t len, int flags) {
    int result = 0;
    for (size_t i = 0; i < len; i += 1) {
        do {
            result = uart_rx_char(&buf[i], flags);
        } while (!result);
    }
    return result;
}

int uart_rx_char(char * c, int flags) {
    unsigned int imask;
    while (1) {
        /* Mask IRQ */
        imask = irq_mask(-1);
        irq_mask(imask | IRQ_UART);
        if (rxbuf_used > 0) {
            *c = rxbuf[rxbuf_rpos++];
            rxbuf_rpos &= BUFMASK;
            rxbuf_used--;
            // We're using the buffer and now have some space, so re-enable IRQ
            uart_uwc(uart_urc() | RX_IRQ_EN);
            irq_mask(imask); //Re-enable IRQ
            return 1; // Returned successfully out of buffer
        }
        if (flags & UART_NONBLOCK) {
            irq_mask(imask); //Re-enable IRQ
            return 0;
        } else {
            /* Wait for UART IRQ */
            while ((irq_wait() & IRQ_UART) == 0);
            /* Allow it to be servied */
            irq_mask(imask);
        }
    }
    return 0;
}


int uart_tx_char(char c, int flags) {
    unsigned int imask;
    while (1) {
        /* Mask IRQ */
        imask = irq_mask(-1);
        irq_mask(imask | IRQ_UART);
         /* If buffer empty and not sending, bypass the buffer/IRQ */
        if (txbuf_used == 0 && !(uart_urc() & TX_ACTIVE)) {
            uart_utx(c);
            irq_mask(imask);
            return 1;
        }
        /* Put character in driver's buffer if there's space, enable interrupt */
        if (txbuf_used < BUFSIZE) {
            txbuf[txbuf_wpos++] = c;
            txbuf_wpos &= BUFMASK;
            txbuf_used++;
            uart_uwc(uart_urc() | TX_IRQ_EN);
            irq_mask(imask); //Re-enable IRQ
            return 1;
        }
        /* We didn't send, so wait for an IRQ */
        if (flags & UART_NONBLOCK) {
            irq_mask(imask); //Re-enable IRQ
            /* We are non-blocking, so just return */
            return 0;
        } else {
            /* Wait for UART IRQ */
            while ((irq_wait() & IRQ_UART) == 0);
            /* Allow it to be servied */
            irq_mask(imask);
        }
    }
    return 0;
}

int uart_tx_buf(char * buf, size_t len, int flags) {
    int result = 0;
    for (size_t i = 0; (len == 0 && buf[i] != '\0') || i < len; i += 1) {
        /* This will spin only if the buffer is full */
        do {
            result = uart_tx_char(buf[i], flags);
        } while (!result);
    }
    if (flags & UART_DRAIN) {
        uart_tx_drain();
    }
    return result;
}

