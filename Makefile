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
INS_SRCS = $(wildcard $(INS_DIR)/*.c)

BS_OBJS    = $(BS_SRCS:%=$(OBJDIR)/%.o)
BSS2X_OBJS = $(BSS2X_SRCS:%=$(OBJDIR)/%.o)
S2_OBJS    = $(S2_SRCS:%=$(OBJDIR)/%.o)
INS_OBJS   = $(INS_SRCS:%=$(OBJDIR)/%.o) $(OBJDIR)/bootsector.bin.c.o $(OBJDIR)/stage2.bin.c.o $(OBJDIR)/bootsector2x.bin.c.o

OBJGUARD = $(OBJDIR)/guard

TARGET = iyasb

all: build install test

$(OBJGUARD):
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/bootsector
	mkdir -p $(OBJDIR)/bootsector2x
	mkdir -p $(OBJDIR)/stage2
	mkdir -p $(OBJDIR)/installer
	touch $(OBJGUARD)

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
$(OBJDIR)/bootsector.bin: $(OBJDIR)/bootsector.elf
	objcopy -O binary $< $@

$(OBJDIR)/bootsector2x.bin: $(OBJDIR)/bootsector2x.elf
	objcopy -O binary $< $@

$(OBJDIR)/stage2.bin: $(OBJDIR)/stage2.elf
	objcopy -O binary $< $@

$(OBJDIR)/%.bin.c: $(OBJDIR)/%.bin
	@echo "unsigned char `basename -s .bin $<`_data[] = {"  > $@
	cat $< | xxd -i  >> $@
	@echo "};"        >> $@
	@echo "unsigned int `basename -s .bin $<`_len = `cat $< | wc -c`;" >> $@

$(OBJDIR)/%.asm.o: %.asm $(OBJGUARD)
	$(AS) $(ASFLAGS) -o $@ $<

$(OBJDIR)/%.c.o: %.c $(OBJGUARD)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJDIR)/%.c.o: $(OBJDIR)/%.c $(OBJGUARD)
	$(CC) $(CFLAGS) -o $@ $<

$(OBJDIR)/$(S2_DIR)/%.c.o: $(S2_DIR)/%.c $(OBJGUARD)
	$(S2_CC) $(S2_CFLAGS) -o $@ $<

# The C files depend on headers, so any update on any of the headers should
# rebuild everything
$(S2_SRCS): $(wildcard $(INCDIR)/*)
$(INS_SRCS): $(wildcard $(INCDIR)/*)
