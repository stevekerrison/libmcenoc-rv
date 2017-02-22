#ifndef MCENOC_NI_H
#define MCENOC_NI_H

#include <stdlib.h>
#include <stdint.h>


#define OPCODE_NRX 24
#define OPCODE_NTX 25
#define OPCODE_NCT 26
#define MCENOC_NI_MINREG 0
#define MCENOC_NI_MAXREG 7

#define MCENOC_CT_INTERR    (1<<10)
#define MCENOC_CT_RXINTE    (1<<9)
#define MCENOC_CT_RXINTV    (1<<8)
#define MCENOC_CT_TXINTE    (1<<7)
#define MCENOC_CT_TXINTV    (1<<6)
#define MCENOC_CT_RXMARK    (1<<5)
#define MCENOC_CT_TXMARK    (1<<4)
#define MCENOC_CT_TXERR     (1<<3)
#define MCENOC_CT_RXCLM     (1<<2)
#define MCENOC_CT_RXERR     (1<<1)
#define MCENOC_CT_TXCLM     (1<<0)

#if defined(MCENOC_UNSAFE) && !defined(MCENOC_SAFE)
    #define MCENOC_SAFE 0
#elif !defined(MCENOC_SAFE)
    #define MCENOC_SAFE 1
#endif


typedef struct {
    uint8_t tokwidth;
    uint8_t netwidth;
    uint8_t nstages;
    uint8_t stagebits;
    uint8_t midbits;
} mcenoc_ni_config_t;

/* Receive 1--4 bytes */
int mcenoc_ni_rx(uint8_t len, uint32_t *val);

/* Send 1--4 bytes */
int mcenoc_ni_tx(uint8_t len, uint32_t val);

/* Open a connection to specified node via given route */
int mcenoc_ni_open(uint32_t nodeid, uint32_t route);

/* Close an open connection, releasing the route. */
int mcenoc_ni_close(void);

/* Generate and error signal towards the sender, tearing down the route */
int mcenoc_ni_err(void);

/* Read a control register */
int mcenoc_ni_readreg(uint8_t regnum, uint32_t *val);

int mcenoc_ni_writereg(uint8_t regnum, uint32_t val);

#endif //MCENOC_NI_H

