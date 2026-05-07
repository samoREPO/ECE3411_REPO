PROJECT := EtchASketch
DEVICE := AVR128DB48
F_CPU := 16000000UL

XC8_CC := /Applications/microchip/xc8/v3.10/bin/xc8-cc
AVR_OBJCOPY := /Applications/microchip/xc8/v3.10/avr/bin/avr-objcopy
AVR_SIZE := /Applications/microchip/xc8/v3.10/avr/bin/avr-size
IPECMD := /Applications/microchip/mplabx/v6.30/mplab_platform/mplab_ipe/ipecmd.sh
MDB := /Applications/microchip/mplabx/v6.30/mplab_platform/bin/mdb.sh

DFP := $(HOME)/.mchp_packs/Microchip/AVR-Dx_DFP/2.8.343/xc8
UPLOAD_TOOL ?= pkobnano
UPLOAD_INTERFACE ?= UPDI
UPLOAD_SPEED ?= 0.100
CURIOSITY_VOLUME ?= /Volumes/CURIOSITY

BUILD_DIR := build
ELF := $(BUILD_DIR)/$(PROJECT).elf
HEX := $(BUILD_DIR)/$(PROJECT).hex
MDB_LOG_DIR := $(BUILD_DIR)/mdb-log
MDB_CONNECT_SCRIPT := $(BUILD_DIR)/connect.mdb
MDB_PROGRAM_SCRIPT := $(BUILD_DIR)/program.mdb

SOURCES := \
	EtchASketch/main.c \
	EtchASketch/Apps/etch_sketch.c \
	EtchASketch/GFX/gfx.c \
	EtchASketch/Drivers/ADC.c \
	EtchASketch/Drivers/I2C.c \
	EtchASketch/Drivers/SH1106.c \
	EtchASketch/Drivers/TCA.c \
	EtchASketch/Drivers/button.c \
	EtchASketch/Drivers/uart.c \
	EtchASketch/Drivers/uart-avrdx.c

INCLUDES := \
	-IEtchASketch \
	-IEtchASketch/Apps \
	-IEtchASketch/GFX \
	-IEtchASketch/Drivers

CFLAGS := \
	-mcpu=$(DEVICE) \
	-mdfp=$(DFP) \
	-O1 \
	-DF_CPU=$(F_CPU) \
	$(INCLUDES)

.PHONY: all build clean distclean size upload upload-mdb upload-volume upload-ipecmd connect list-tools

all: build

build: $(HEX) size

$(BUILD_DIR)/.dir:
	mkdir -p $(BUILD_DIR)
	touch $(BUILD_DIR)/.dir

$(ELF): $(SOURCES) | $(BUILD_DIR)/.dir
	$(XC8_CC) $(CFLAGS) $(SOURCES) -o $(ELF)

$(HEX): $(ELF)
	$(AVR_OBJCOPY) -O ihex -R .eeprom $(ELF) $(HEX)

size: $(ELF)
	$(AVR_SIZE) $(ELF)

connect:
	mkdir -p $(MDB_LOG_DIR)
	printf 'Device $(DEVICE)\nSet interface $(UPLOAD_INTERFACE)\nhwtool $(UPLOAD_TOOL) -p\nquit\n' > $(MDB_CONNECT_SCRIPT)
	$(MDB) -d $(MDB_LOG_DIR) $(MDB_CONNECT_SCRIPT)

list-tools:
	mkdir -p $(MDB_LOG_DIR)
	printf 'hwtool\nquit\n' > $(BUILD_DIR)/tools.mdb
	$(MDB) -d $(MDB_LOG_DIR) $(BUILD_DIR)/tools.mdb

upload: upload-mdb

upload-mdb: $(HEX)
	mkdir -p $(MDB_LOG_DIR)
	printf 'Device $(DEVICE)\nSet interface $(UPLOAD_INTERFACE)\nhwtool $(UPLOAD_TOOL) -p\nProgram $(abspath $(HEX))\nquit\n' > $(MDB_PROGRAM_SCRIPT)
	$(MDB) -d $(MDB_LOG_DIR) $(MDB_PROGRAM_SCRIPT)

upload-volume: $(HEX)
	test -d "$(CURIOSITY_VOLUME)"
	cp $(HEX) "$(CURIOSITY_VOLUME)/"
	sync

upload-ipecmd: $(HEX)
	$(IPECMD) -P$(DEVICE) -TPPKOB4 -ORI$(UPLOAD_INTERFACE) -ORS$(UPLOAD_SPEED) -F$(HEX) -M -Y

clean:
	rm -rf $(BUILD_DIR)

distclean: clean
	rm -rf .vs EtchASketch/Debug
