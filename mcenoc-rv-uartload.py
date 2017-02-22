#!/usr/bin/env python3
"""
    mcenoc-rv-uartload.py - MCENoC RV UART Loader
    
    A loader script for RV32 ELF or binary files onto the MCENoC system
    via UART.
    
    Copyright (c) 2016-2017 Steve Kerrison
    License: ISC License
    
    Usage:
        mcenoc-rc-uartload.py <file>
    
    Load given program file from <file>. Use "-" for stdin.
"""

import serial, sys, os, binascii, time, struct

if __name__ == "__main__":
    if sys.argv[1] == '-':
        binfile = sys.stdin.buffer.read()
    else:
        with open(sys.argv[1], "rb") as f:
            binfile = f.read()

    ser = serial.Serial(
        port='/dev/ttyUSB0',
        baudrate=115200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_TWO,
        bytesize=serial.EIGHTBITS,
        timeout=0.001,
        write_timeout=0
    )

    # Consume all existing data
    #print (ser.read(2**10).decode("ascii"))


    fsize = len(binfile)

    # Send the magic word
    ser.write(b"mcen")
    r = ser.read(4)
    if r != b"go\r\n":
        raise IOError("MCENoC-RV system booloader gave invalid response: {}".format(r))

    ser.write((fsize-4).to_bytes(4,'little'))

    crc = binascii.crc32(binfile[4:])
    ser.write(binfile[4:])
    ser.write(crc.to_bytes(4, 'little'))

