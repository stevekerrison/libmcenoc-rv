#include <stdlib.h>
#include <errno.h>
#include "irq-ops.h"
#include "mcenoc-ni.h"

mcenoc_ni_config_t mcenoc_ni_config = {
    .tokwidth = 8,
    .netwidth = 8,
    .nstages  = 5,
    .stgbits  = 2,
    .midbits  = 2
};

/* Receive 1--4 bytes */
int mcenoc_ni_rx(uint8_t len, uint32_t *val) {
    if (MCENOC_SAFE) {
        if (len < 1 || len > 4) {
            return -EINVAL;
        }
    }
    __asm__ __volatile__("custom0 %0,0,%1,%2":"=r"(*val):"i"(len-1),"i"(OPCODE_NRX));
    return len;
}

/* Send 1--4 bytes */
int mcenoc_ni_tx(uint8_t len, uint32_t val) {
    if (MCENOC_SAFE) {
        if (len < 1 || len > 4) {
            return -EINVAL;
        }
    }
    __asm__ __volatile__("custom0 zero,%0,%1,%2"::"r"(val),"i"(len-1),"i"(OPCODE_NTX));
    return len;
}
/* Open a connection to specified node via given route */
int mcenoc_ni_open(uint32_t nodeid, uint32_t route) {
    uint32_t regval;
    mcenoc_ni_readreg(0, &regval);
    if (MCENOC_SAFE) {
        if (regval & MCENOC_CT_TXCLM) {
            return -EBUSY;
        } else if (regval & MCENOC_CT_TXERR) {
            return -EIO;
        }
    }
    /* Claim interface */
    regval |= MCENOC_CT_TXCLM;
    mcenoc_ni_writereg(0, regval);
    /* Send route header */
    unsigned routemask = (1 << mcenoc_ni_config.stgbits)-1;
    /* Inbound stages */
    for (size_t i = 0; i < (nstages-1)<<1; i += 1) {
        mcenoc_ni_tx(1, route &= routemask);
        route >>= mcenoc_ni_config.stgbits;
    }
    /* Middle stage */
    mcenoc_ni_tx(1, route & (1 << mcenoc_ni_config.midbits)-1);
    route >>= mcenoc_ni_config.mcenoc_ni_config.midbits;
    /* Outbound stages */
    for (size_t i = 0; i < (nstages-1)<<1; i += 1) {
        mcenoc_ni_tx(1, route &= routemask);
        route >>= mcenoc_ni_config.stgbits;
    }
    mcenoc_ni_readreg(0, &regval);
    if (regval & MCENOC_CT_TXERR) {
        return -EIO;
    }
    return 0;
}

/* Close an open connection, releasing the route. */
int mcenoc_ni_close(void) {
    uint32_t regval;
    mcenoc_ni_readreg(0, &regval);
    if (MCENOC_SAFE) {
        if (!(regval & MCENOC_CT_TXCLM)) {
            return -EIO;
        }
    }
    regval ^= MCENOC_CT_TXCLM;
    mcenoc_ni_writereg(0, regval);
    return -ENOSYS;
}

/* Generate an error signal towards the sender, tearing down the route */
int mcenoc_ni_err(void) {
    uint32_t regval;
    mcenoc_ni_readreg(0, &regval);
    if (MCENOC_SAFE) {
        if (regval & MCENOC_CT_RXERR) {
            return -EBUSY;
        } else if (regval & MCENOC_CT_RXCLM) {
            return -EIO;
        }
    }
    regval |= MCENOC_CT_RXERR;
    mcenoc_ni_writereg(0, regval);
    return 0;
}

/* Read a control register */
int mcenoc_ni_readreg(uint8_t regnum, uint32_t *val) {
    if (regnum > MCENOC_NI_MAXREG) {
        return -EINVAL;
    }
    __asm__ __volatile__("custom0 %0,0,%1,%2":"=r"(*val):"i"(regnum),"i"(OPCODE_NCT));
    return 0;
}

int mcenoc_ni_writereg(uint8_t regnum, uint32_t val) {
    if (regnum > MCENOC_NI_MAXREG) {
        return -EINVAL;
    } else if (regnum > 0) {
        return -EPERM;
    }
    __asm__ __volatile__("custom0 zero,%0,%1,%2"::"r"(val),"i"((1 << 23) | regnum),"i"(OPCODE_NCT));
    return 0;
}

