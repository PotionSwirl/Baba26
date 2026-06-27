CC = gcc
CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPPER -I/usr/include/efi -I/usr/include/efi/x86_64 -Wall
LFLAGS = -nostdlib -shared -Wl,-znocombreloc -Wl,-Bsymbolic -Wl,-z,notext -Wl,-z,common-page-size=4096 -Wl,-z,max-page-size=4096 -Wl,-T,/usr/lib/elf_x86_64_efi.lds /usr/lib/crt0-efi-x86_64.o 

all: Find Baba26/Bootx64.efi Sign Run

C_SOURCES := $(shell find Source -name "*.c")
OBJS := $(patsubst Source/%.c, Build/Objects/%.o, $(C_SOURCES))

Find:
	find Source -type d | sed 's|Source|Build/Objects|' | xargs mkdir -p
	rm -r Build/Objects/Include

Build/Objects/%.o: Source/%.c
	$(CC) $(CFLAGS) -c $< -o $@

Baba26/Bootx64.efi: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o Build/Objects/Bootx64.so -L/usr/lib -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel \
        -j .rela -j .reloc -j .sbat --target=efi-app-x86_64 \
        Build/Objects/Bootx64.so Build/Objects/Bootx64.efi

Sign:
	openssl req -new -x509 -newkey rsa:2048 -nodes -config Build/Objects/MOK.cnf -keyout Build/Objects/Key.key -out Build/Objects/Certificate.crt -days 365 -subj "/CN=Baba26/"
	openssl x509 -in Build/Objects/Certificate.crt -out Build/Objects/Certificate.der -outform DER
	sbsign --key Build/Objects/Key.key --cert Build/Objects/Certificate.crt --output Build/Objects/Bootx64.efi Build/Objects/Bootx64.efi

Run:
	mkdir -p Build/Virtual-Drive/EFI/Baba26/Boot
	cp Build/Objects/Bootx64.efi Build/Virtual-Drive/EFI/Baba26/Boot/Grubx64.efi
	cp Build/Objects/Certificate.der Build/Virtual-Drive/EFI/Baba26/Boot/Certificate.der
	
	qemu-system-x86_64 \
  	-bios /usr/share/ovmf/OVMF.fd \
  	-drive file=fat:rw:Build/Virtual-Drive,format=raw

USB:
	sudo mount -t drvfs D: /mnt/d
	mkdir -p /mnt/d/Setup-Files
	cp -r Build/Virtual-Drive/EFI/Baba26/Boot/. /mnt/d/Setup-Files
	sudo umount /mnt/d

Commit:
	git remote set-url origin https://github.com/PotionSwirl/Baba26.git
	git add .
	git commit -m "$(MSG)"
	git push
