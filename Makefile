.DEFAULT_GOAL := all

CC=riscv32-unknown-elf-gcc
AR=riscv32-unknown-elf-gcc-ar
CFLAGS=-march=RV32IMCXcustom -m32 -Wall -static -std=c11 -Os -flto
# -DSYSCALL_DEBUG

OBJECTS=uart.o irq-ops.o isr.o syscall.o riscv.ld

%.o: %.S
	$(CC) $(CFLAGS) -c $<

%.o: %.S %.h
	$(CC) $(CFLAGS) -c $<

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

libmcenoc-rv.a: $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

.PHONY: clean
clean:
	rm -f *.o *.a

.PHONY: all
all: libmcenoc-rv.a
