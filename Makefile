CC	= i686-elf-gcc
AS	= nasm
LD	= i686-elf-ld

OBJDIR	:= $(or $(OBJDIR),$(CURDIR)/obj)
INCDIR	:= $(or $(INCDIR),$(CURDIR)/include)

DEVFILE	:= $(or $(DEVFILE),dev.o)
DEVDIR	:= $(or $(DEVDIR),dev.d)

ifeq ($(strip $(DEBUG)), Y)
DEBUG	= -D _DEBUG
OBJDIR	:= $(OBJDIR)/debug
else
DEBUG	=
OBJDIR	:= $(OBJDIR)/default
endif

CFLAGS	:= -m32 -c -g -I$(INCDIR) -Wall -Wno-builtin-declaration-mismatch $(DEBUG)
ASFLAGS	:= -f elf32 -F dwarf -g -Ox $(ASFLAGS) $(DEBUG)

CFLAGS	:= $(strip $(CFLAGS))
LDFLAGS	:= $(strip $(LDFLAGS))
ASFLAGS	:= $(strip $(ASFLAGS))

# Bootsector sources
BSDIR	:= $(or $(BSDIR),bootsector)
BSSRCS	= $(wildcard $(BSDIR)/*.asm)

# Bootsector (for partitioned media) sources
BS2XDIR	:= $(or $(BS2XDIR),bootsector2x)
BSS2XSRCS	= $(wildcard $(BS2XDIR)/*.asm)

# Stage2 sources
S2DIR	:= $(or $(S2DIR),stage2)
S2SRCS	= $(wildcard $(S2DIR)/*.asm) $(wildcard $(S2DIR)/*.c)

BSOBJS	= $(BSSRCS:%=$(OBJDIR)/%.o)
BSS2XOBJS	= $(BSS2XSRCS:%=$(OBJDIR)/%.o)
S2OBJS	= $(S2SRCS:%=$(OBJDIR)/%.o)

all: build install test

$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/bootsector
	mkdir -p $(OBJDIR)/bootsector2x
	mkdir -p $(OBJDIR)/stage2

build: $(OBJDIR) $(OBJDIR)/bootsector.bin $(OBJDIR)/bootsector2x.bin $(OBJDIR)/stage2.bin

install: build $(DEVFILE) $(DEVDIR)
	dd if=$(OBJDIR)/bootsector.bin of=$(DEVFILE) bs=1 conv=notrunc count=3
	dd if=$(OBJDIR)/bootsector.bin of=$(DEVFILE) bs=1 conv=notrunc count=344 seek=96 skip=96
	cp $(OBJDIR)/stage2.bin $(DEVDIR)/stage2
	sync $(DEVFILE)
	sync $(DEVDIR)/stage2

# An existing media is needed to test stuff (duh)
test: install
clean:
	rm -r $(OBJDIR)

# This creates just the elfs, still with debugging symbols
$(OBJDIR)/bootsector.elf: $(BSOBJS)
	$(LD) -T $(BSDIR)/link.ld -o $@ $^

$(OBJDIR)/bootsector2x.elf: $(BSS2XOBJS)
	$(LD) -T $(BS2XDIR)/link.ld -o $@ $^

$(OBJDIR)/stage2.elf: $(S2OBJS)
	$(LD) -T $(S2DIR)/link.ld -o $@ $^

# Extract the actual binary from the elfs
$(OBJDIR)/bootsector.bin: $(OBJDIR)/bootsector.elf
	objcopy -O binary $< $@

$(OBJDIR)/bootsector2x.bin: $(OBJDIR)/bootsector2x.elf
	objcopy -O binary $< $@

$(OBJDIR)/stage2.bin: $(OBJDIR)/stage2.elf
	objcopy -O binary $< $@

# Compile every .asm and .c file
$(OBJDIR)/%.asm.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

$(OBJDIR)/%.c.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

# Some C files depend on headers, so any update on any of the headers should
# rebuild everything
$(S2SRCS): $(wildcard $(INCDIR)/*)
