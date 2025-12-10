SRC_DIR ?= ./
OBJ_DIR ?= ./
SOURCES ?= $(shell find $(SRC_DIR) -name '*.c' -or -name '*.S')
OBJECTS ?= $(addsuffix .o, $(basename $(notdir $(SOURCES))))
LINKER ?= $(SRC_DIR)/dtekv-script.lds

TOOLCHAIN ?= riscv32-unknown-elf-
BASE_FLAGS ?= -Wall -nostdlib -mabi=ilp32 -march=rv32imzicsr -fno-builtin

OPT ?= -O3

CFLAGS = $(BASE_FLAGS) $(OPT)


build: clean main.bin

build-O0:
	$(MAKE) build OPT=-O0
	mv main.bin main-O0.bin
	mv main.elf main-O0.elf


run-O0: build-O0
	$(MAKE) run FILE_TO_RUN=$(CURDIR)/main-O0.bin


build-O3:
	$(MAKE) build OPT=-O3
	mv main.bin main-O3.bin
	mv main.elf main-O3.elf


run-O3: build-O3
	$(MAKE) run FILE_TO_RUN=$(CURDIR)/main-O3.bin


main.elf: 
	$(TOOLCHAIN)gcc -c $(CFLAGS) $(SOURCES)
	$(TOOLCHAIN)ld -o $@ -T $(LINKER) $(filter-out boot.o, $(OBJECTS)) softfloat.a


main.bin: main.elf  
	$(TOOLCHAIN)objcopy --output-target binary $< $@
	$(TOOLCHAIN)objdump -D $< > $<.txt

clean:
	rm -f *.o *.elf *.bin *.txt


TOOL_DIR ?= ./tools
run: main.bin
	make -C $(TOOL_DIR) "FILE_TO_RUN=$(CURDIR)/$<"
