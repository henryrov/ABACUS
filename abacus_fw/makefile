CC=arm-none-eabi-gcc
CFLAGS=--specs=picolibc.specs -mcpu=cortex-m0 -mthumb -lgcc -Wall -Wextra
TARGET_FILES=abacus.elf abacus.o sprites/sprites.h sprites/sprites.c sprites/sprites.o math/descent.o math/token.o math/expression.o display.o interface.o

.PHONY=all spritefiles clean

all: abacus.elf

abacus.elf: abacus.o interface.o display.o battery.o sprites/sprites.o math/descent.o math/token.o math/expression.o
	$(CC) $(CFLAGS) -T linker_script.ld -o $@ $^

abacus.o: abacus.c spritefiles
	$(CC) $(CFLAGS) -c -o $@ $<

interface.o: interface.c spritefiles
	$(CC) $(CFLAGS) -c -o $@ $<

display.o: display.c spritefiles
	$(CC) $(CFLAGS) -c -o $@ $<

battery.o: battery.c spritefiles
	$(CC) $(CFLAGS) -c -o $@ $<

math/descent.o: math/descent.c spritefiles
	$(CC) $(CFLAGS) -c -o $@ $<

math/token.o: math/token.c spritefiles
	$(CC) $(CFLAGS) -c -o $@ $<

math/expression.o: math/expression.c spritefiles
	$(CC) $(CFLAGS) -c -o $@ $<

sprites/sprites.o: spritefiles
	$(CC) $(CFLAGS) -c -o $@ sprites/sprites.c

spritefiles:
	cd sprites && QSPRITELY/qspritely.sh *.raw

flash:
	openocd -f ft232h_ocd.cfg -c init -c "reset halt" -c "flash write_image erase abacus.elf 0x08000000" -c "reset" -c shutdown

clean:
	$(RM) $(TARGET_FILES)
