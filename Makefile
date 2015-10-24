AS:=i686-elf-as
CC:=i686-elf-gcc

CFLAGS:=-ffreestanding -O2 -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs
CPPFLAGS:=
LIBS:=-lgcc

OBJS:=\
boot.o \
kernel.o \

all: jenOS.bin

.PHONEY: all clean iso run-qemu

jenOS.bin: $(OBJS) linker.ld
	$(CC) -T linker.ld -o $@ $(CFLAGS) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

%.o: %.s
	$(AS) $< -o $@

clean:
	rm -rf isodir
	rm -f jenOS.bin jenOS.iso $(OBJS)

iso: jenOS.iso

isodir isodir/boot isodir/boot/grub:
	mkdir -p $@

isodir/boot/jenOS.bin: jenOS.bin isodir/boot
	cp $< $@

isodir/boot/grub/grub.cfg: grub.cfg isodir/boot/grub
	cp $< $@

jenOS.iso: isodir/boot/jenOS.bin isodir/boot/grub/grub.cfg
	grub-mkrescue -o $@ isodir

run-qemu: jenOS.iso
	qemu-system-i386 -cdrom jenOS.iso
