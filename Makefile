SHELL := bash
.ONESHELL:
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules


S2_CC = clang -target i386-pc-none
CC    = clang
AS    = nasm
LD    = ld.lld

BS_DIR   := $(or $(BS_DIR),bootsector)
BS2X_DIR := $(or $(BS2X_DIR),bootsector2x)
S2_DIR   := $(or $(S2_DIR),stage2)
INS_DIR  := $(or $(INS_DIR),installer)

OBJDIR    := $(or $(OBJDIR),obj)
S2_INCDIR := $(or $(INCDIR),$(S2_DIR)/include)

DEVFILE := $(or $(DEVFILE),dev.o)
DEVDIR  := $(or $(DEVDIR),dev.d)

ifeq ($(strip $(DEBUG)), Y)
DEBUG   = -D _DEBUG
OBJDIR := $(OBJDIR)/debug
else
DEBUG   =
OBJDIR := $(OBJDIR)/default
endif

CFLAGS     := -O3 -c -g -I$(S2_INCDIR) -Wall -Werror
S2_CFLAGS  := -O3 -c -g -I$(S2_INCDIR) -ffreestanding -Wall -Werror -Wno-missing-braces -mno-sse $(DEBUG)
ASFLAGS    := -f elf32 -F dwarf -g -Ox $(ASFLAGS) $(DEBUG)

CFLAGS    := $(strip $(CFLAGS))
S2_CFLAGS := $(strip $(S2_CFLAGS))
ASFLAGS   := $(strip $(ASFLAGS))

# Bootsector sources
BS_SRCS = $(wildcard $(BS_DIR)/*.asm)

# Bootsector (for partitioned media) sources
BSS2X_SRCS = $(wildcard $(BS2X_DIR)/*.asm)

# Stage2 sources
S2_SRCS = $(wildcard $(S2_DIR)/*.asm) $(wildcard $(S2_DIR)/*.c)

# Installer sources
INS_SRCS = $(wildcard $(INS_DIR)/*.c) bootsector.bin stage2.bin bootsector2x.bin

HEADERS := $(wildcard $(S2_INCDIR)/*.h)

BS_OBJS    = $(BS_SRCS:%=$(OBJDIR)/%.o)
BSS2X_OBJS = $(BSS2X_SRCS:%=$(OBJDIR)/%.o)
S2_OBJS    = $(S2_SRCS:%=$(OBJDIR)/%.o)
INS_OBJS   = $(INS_SRCS:%=$(OBJDIR)/%.o)

OBJGUARD = $(OBJDIR)/guard

TARGET = iyasb

all: build install test

build: $(TARGET)

install: build $(DEVFILE) $(DEVDIR)
	mkdir -p $(DEVDIR)/boot
	./$(TARGET) $(DEVFILE) $(DEVDIR)
	sync $(DEVFILE)
	sync -f $(DEVDIR)/.

# An existing media is needed to test stuff (duh)
test: install
clean:
	rm -r $(OBJDIR)

$(TARGET): $(INS_OBJS)
	$(CC) -o $@ $^

# This creates just the elfs, still with debugging symbols
$(OBJDIR)/bootsector.elf: $(BS_OBJS)
	$(LD) -T $(BS_DIR)/link.ld -o $@ $^

$(OBJDIR)/bootsector2x.elf: $(BSS2X_OBJS)
	$(LD) -T $(BS2X_DIR)/link.ld -o $@ $^

$(OBJDIR)/stage2.elf: $(S2_OBJS)
	$(LD) -T $(S2_DIR)/link.ld -o $@ $^

# Extract the actual binary from the elfs
$(OBJDIR)/%.bin: $(OBJDIR)/%.elf
	objcopy -O binary $< $@

# Transform the binaries into usable object files
$(OBJDIR)/%.bin.o.tmp: $(OBJDIR)/%.bin
	ld -r -b binary -o $@ $<

$(OBJDIR)/%.bin.o: $(OBJDIR)/%.bin.o.tmp
	TRANSFORMED_PREFIX="_binary_$$(echo -n "$(basename $(basename $<))" | tr -c '[A-Za-z0-9]' _)"
	NEW_PREFIX="_resource_$$(echo -n "$(basename $(notdir $@))" | tr -c '[A-Za-z0-9]' _)"
	objcopy --redefine-sym "$$TRANSFORMED_PREFIX"_start="$$NEW_PREFIX"_start \
			--redefine-sym "$$TRANSFORMED_PREFIX"_end="$$NEW_PREFIX"_end \
			--redefine-sym "$$TRANSFORMED_PREFIX"_size="$$NEW_PREFIX"_size \
			$< $@

$(OBJDIR)/%.asm.o: %.asm $(OBJGUARD)
	$(AS) $(ASFLAGS) -o $@ $<

$(OBJDIR)/%.c.o: %.c $(HEADERS) $(OBJGUARD)
	$(CC) $(CFLAGS) -o $@ $<

$(OBJDIR)/$(S2_DIR)/%.c.o: $(S2_DIR)/%.c $(HEADERS) $(OBJGUARD)
	$(S2_CC) $(S2_CFLAGS) -o $@ $<

$(OBJGUARD):
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/bootsector
	mkdir -p $(OBJDIR)/bootsector2x
	mkdir -p $(OBJDIR)/stage2
	mkdir -p $(OBJDIR)/installer
	touch $(OBJGUARD)
