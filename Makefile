AS:=as
LD:=ld
CC:=gcc
OBJCOPY:=objcopy
NAME:=jenOS

CFLAGS:=-I/usr/include/efi -fpic -ffreestanding -fno-stack-protector \
        -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args
CPPFLAGS:=
LDFLAGS:=-shared -Bsymbolic -L/usr/lib/ -T /usr/lib/elf_x86_64_efi.lds /usr/lib/crt0-efi-x86_64.o
LIBS:=-lgnuefi -lefi

OBJS:=\
kmain.o

BUILDIR:=/tmp/jenos_build

# On Arch-Linux it's part of the 'edk2-ovmf' package
BIOS?=/usr/share/edk2-ovmf/x64/OVMF.fd

all: $(NAME).bin

.PHONEY: all clean run-qemu

$(NAME).vmdk: $(NAME).img
	qemu-img convert -O vmdk $(NAME).img $(NAME).vmdk

$(NAME).img: $(NAME).efi
	dd if=/dev/zero of=$(NAME).img bs=512 count=100000
	mkfs.fat -F16 $(NAME).img
	rm -rf $(BUILDIR)
	mkdir $(BUILDIR)
	sudo mount $$(sudo losetup -f --show $(NAME).img) $(BUILDIR)
	sudo mkdir -p $(BUILDIR)/EFI/BOOT
	sudo cp $(NAME).efi $(BUILDIR)/EFI/BOOT/BOOTX64.efi
	sudo umount $(BUILDIR)
	sudo losetup -D

$(NAME).efi: $(NAME).so
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 $(NAME).so $(NAME).efi

$(NAME).so: $(OBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

%.o: %.s
	$(AS) $< -o $@

clean:
	rm -rf build
	rm -f $(NAME).vmdk $(NAME).img $(NAME).efi $(NAME).so \
	      $(NAME).bin $(NAME).iso $(OBJS)


run-qemu: $(NAME).vmdk
	qemu-system-x86_64 --bios $(BIOS) \
	                   -net none \
	                   -usb \
	                   -drive if=none,id=usbstick,file=$(NAME).vmdk \
	                   -device usb-storage,drive=usbstick
