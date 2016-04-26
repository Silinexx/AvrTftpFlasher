################################################################################
# Atmega328p firmware updater over TFTP protocol.
# 
# Copyright (C) 2015-2016 Silinexx LLC, Lviv, Ukraine.
# 
# These coded instructions, statements, and computer programs
# (here and after SOFTWARE) are copyrighted by Silinexx LLC
# and published as open-source software. This meant that you can use
# and/or modify it only for personal purposes. Any comertial usage
# and/or publishing of this SOFTWARE in any form, in whole or in part,
# should not being done without the specific, prior written permission
# of Silinexx LLC.
# 
# This program is distributed in the hope that it will be useful but
# WITHOUT ANY WARRANTY, even without warranty of
# FITNESS FOR A PARTICULAR PURPOSE.
################################################################################

################################################################################
# Author: Andrian Yablonskyy
# Date: 2015-07-22
# Email: andrian.yablonskyy@silinexx.com
# File: Makefile
################################################################################
 
 
################################################################################
# Atmega328p + enc28j60
################################################################################

BASEDIR = $(shell pwd)

### Project name (also used for output file name)
PROJECT = TftpFlasher

DEVICE = atmega328p
F_OSC = 16000000
PORT = usb
PROGRAMMER = usbasp

# WE ARE USING THE FOLLOWING SETTINGS
FUSES = avrdude -c ${PROGRAMMER} -p ${DEVICE} -U lfuse:w:0xff:m \
	-U hfuse:w:0xd8:m -U efuse:w:0xFF:m
AVRDUDE = avrdude -e -c ${PROGRAMMER} -p ${DEVICE} \
	-U flash:w:$(PROJECT).hex -B 1
RESET   = avrdude -c ${PROGRAMMER} -p ${DEVICE}

### Source files and search directory
CSRC = common.c \
    enc28j60.c \
    ipstack.c \
    tftp.c \
    flash.c \
    boot.c

################################################################################
#
# Don't change anything below
#
################################################################################
### Optimization level (0, 1, 2, 3, 4 or s)
OPTIMIZE = s

### C Standard level (c89, gnu89, c99 or gnu99)
CSTD = gnu99

### Warning control
WARNINGS = all extra

### Output directory
OBJDIR = ${BASEDIR}/obj
BINDIR = ${BASEDIR}/bin

### Output file format (ihex, bin or both) and debugger type
OUTPUT = both
DEBUG = # dwarf-2

### Programs to build porject
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm

# Define all object files
COBJ = $(CSRC:.c=.o) 
COBJ := $(addprefix $(OBJDIR)/,$(COBJ))
PROJECT := $(OBJDIR)/$(PROJECT)

FLASH_SIZE_KB        = 32
BOOT_SECTION_SIZE_KB = 4
BOOT_START           = 0x$(shell echo "obase=16; \
	($(FLASH_SIZE_KB) - $(BOOT_SECTION_SIZE_KB)) * 1024" | bc)

DEFS = F_CPU=$(F_OSC) 
DEFS += BOOT_START_ADDR=$(BOOT_START)UL

CFLAGS = 
CFLAGS += -O$(OPTIMIZE)
CFLAGS += -funsigned-char
CFLAGS += -funsigned-bitfields
CFLAGS += -ffunction-sections
CFLAGS += -fno-inline-small-functions
CFLAGS += -fpack-struct
CFLAGS += -fshort-enums
CFLAGS += -fno-strict-aliasing
CFLAGS += -Wall
CFLAGS += -Wstrict-prototypes
#CFLAGS += -mshort-calls
#CFLAGS += -fno-unit-at-a-time
#CFLAGS += -Wundef
#CFLAGS += -Wunreachable-code
#CFLAGS += -Wsign-compare
CFLAGS += -Wa,-adhlns=$(<:%.c=$(OBJDIR)/%.lst)
CFLAGS += -std=$(CSTD)
CFLAGS += -mmcu=$(DEVICE)
CFLAGS += $(addprefix -D,$(DEFS))

MATH_LIB = -lm

LDFLAGS  = -Wl,-Map=$(PROJECT).map,--cref
LDFLAGS += -Wl,--relax
LDFLAGS += -Wl,--gc-sections
LDFLAGS += $(MATH_LIB)
# Program BOOT section
LDFLAGS += -Wl,--section-start=.text=$(BOOT_START)


# Default target.
all: build size

ifeq ($(OUTPUT),ihex)
build: elf hex lst sym eep
hex: $(PROJECT).hex
else
ifeq ($(OUTPUT),binary)
build: elf bin lst sym eep
bin: $(PROJECT).bin
else
ifeq ($(OUTPUT),both)
build: elf hex bin lst sym eep
hex: $(PROJECT).hex
bin: $(PROJECT).bin
else
$(error "Invalid format: $(OUTPUT)")
endif
endif
endif

elf: $(PROJECT).elf
lst: $(PROJECT).lst 
sym: $(PROJECT).sym
eep: $(PROJECT).eep


# Create final output file (.hex or .bin) from ELF output file.
%.hex: %.elf
	@echo
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock $< $@
	@ [ -d $(BINDIR) ] || mkdir -p $(BINDIR)/
	@cp -f $@ $(BINDIR)/

# Create binary outbut file from ELF output file.
%.bin: %.elf
	@echo
	$(OBJCOPY) -O binary -R .eeprom -R .fuse -R .lock $< $@
	@ [ -d $(BINDIR) ] || mkdir -p $(BINDIR)/
	cp -f $@ $(BINDIR)/

# Create extended listing file from ELF output file.
%.lst: %.elf
	@echo
	$(OBJDUMP) -h -S -C $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	$(NM) -n $< > $@

# Create an eeprom file from ELF output file.
%.eep: %.elf
	@echo
	@echo "Creating load file for EEPROM:" $@
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 --no-change-warnings -O ihex $< $@ || exit 0
	
# Display size of file.
size:
	@echo
	$(SIZE) -C --mcu=$(DEVICE) $(PROJECT).elf

# Link: create ELF output file from object files.
%.elf:  $(COBJ)
	@echo
	@echo Linking...
	$(CC) $(CFLAGS) $(COBJ) --output $@ $(LDFLAGS)

# Compile: create object files from C source files. ARM or Thumb(-2)
$(COBJ) : $(OBJDIR)/%.o : %.c
	@echo
	@echo $< :
	$(CC) -c $(CFLAGS) $< -o $@

# Target: clean project.
clean:
	@echo
	rm -f -r $(OBJDIR) | exit 0

# Include the dependency files.
-include $(shell mkdir $(OBJDIR) 2>/dev/null) $(wildcard $(OBJDIR)/*.d)

fuses:
	@echo
	$(FUSES) 

upload:
	@echo
	$(AVRDUDE) 

reset:
	@echo
	$(RESET)
