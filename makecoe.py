#!/usr/bin/env python3
#
# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.

from sys import argv

binfile = argv[1]
coefile = argv[2]

with open(binfile, "rb") as f:
    bindata = f.read()



with open(coefile, "wb") as f:
    f.write(b"""memory_initialization_radix=16;
memory_initialization_vector=\n""")
    for i in range(len(bindata)//4):
        w = bindata[4*i : 4*i+4]
        f.write(b"%02x%02x%02x%02x\n" % (w[3], w[2], w[1], w[0]))
