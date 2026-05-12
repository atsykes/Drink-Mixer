DEVICE     = atmega1284p
CLOCK      = 8000000
PROGRAMMER = -c usbasp -P usb -B 10
OBJECTS    = main.o lcd_i2c.o adc.o drink.o sonar.o queue.o orders.o rfid.o
# MAIN PROG: main.o lcd_i2c.o adc.o drink.o sonar.o queue.o orders.o rfid.o
# MENU TEST: drink.o lcd_i2c.o adc.o test/menu_display_test.o
# ADC TEST: lcd_i2c.o drink.o adc.o queue.o
# SONAR TEST: test/sonar_sensor_test.o lcd_i2c.o adc.o sonar.o
# RFID TEST: main_lcd_rfid.o lcd_i2c.o rfid.o
FUSES      = -U hfuse:w:0xd9:m -U lfuse:w:0xe2:m -U efuse:w:0xff:m

# Tune the lines below only if you know what you are doing:

AVRDUDE = /opt/homebrew/bin/avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:main.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID main.hex

clean:
	rm -f main.hex main.elf $(OBJECTS)

# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --format=avr --mcu=$(DEVICE) main.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
