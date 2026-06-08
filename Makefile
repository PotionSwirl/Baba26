CC = gcc
CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPPER -I/usr/include/efi -I/usr/include/efi/x86_64 -Wall
LFLAGS = -nostdlib -shared -Wl,-znocombreloc -Wl,-Bsymbolic -Wl,-z,notext -Wl,-T,/usr/lib/elf_x86_64_efi.lds /usr/lib/crt0-efi-x86_64.o

all: Find Bootx64.efi Sign Run

C_SOURCES := $(shell find Source -name "*.c")
OBJS := $(patsubst Source/%.c, Build/Objects/%.o, $(C_SOURCES))

Find:
	find Source -type d | sed 's|Source|Build/Objects|' | xargs mkdir -p
	# rm -r Build/Objects/include

Build/Objects/%.o: Source/%.c
	$(CC) $(CFLAGS) -c $< -o $@

Bootx64.efi: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o Build/Objects/Bootx64.so -L/usr/lib -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel \
        -j .rela -j .reloc -j .sbat --target=efi-app-x86_64 \
		--target=efi-app-x86_64 \
        Build/Objects/Bootx64.so Build/Objects/Bootx64.efi

Sign:
	openssl req -new -x509 -newkey rsa:2048 -nodes -config Build/Objects/mok.cnf -keyout Build/Objects/key.key -out Build/Objects/certificate.crt -days 365 -subj "/CN=Baba26/"
	openssl x509 -in Build/Objects/certificate.crt -out Build/Objects/certificate.der -outform DER
	sbsign --key Build/Objects/key.key --cert Build/Objects/certificate.crt --output Build/Objects/Bootx64.efi Build/Objects/Bootx64.efi

Run:
	mkdir -p Build/"Virtual USB Root"/EFI/Boot
	cp Build/Objects/Bootx64.efi Build/"Virtual USB Root"/EFI/Boot/grubx64.efi
	cp Build/Objects/shimx64.efi Build/"Virtual USB Root"/EFI/Boot/bootx64.efi
	cp Build/Objects/mmx64.efi Build/"Virtual USB Root"/EFI/Boot/mmx64.efi
	cp Build/Objects/certificate.der Build/"Virtual USB Root"/EFI/Boot/certificate.der
	
	qemu-system-x86_64 \
  	-bios /usr/share/ovmf/OVMF.fd \
  	-drive file=fat:rw:Build/"Virtual USB Root",format=raw

USB:
	sudo mount -t drvfs D: /mnt/d
	mkdir -p /mnt/d/EFI/BOOT
	cp -r Build/"Virtual USB Root"/. /mnt/d/
	rm /mnt/d/NvVars
	sudo umount /mnt/d

Commit:
	git add .
	git commit -m "$(MSG)"
	git push
