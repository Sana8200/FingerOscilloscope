SRC_DIR ?= ./
OBJ_DIR ?= ./
SOURCES ?= $(shell find $(SRC_DIR) -name '*.c' -or -name '*.S')
OBJECTS ?= $(addsuffix .o, $(basename $(notdir $(SOURCES))))
LINKER ?= $(SRC_DIR)/dtekv-script.lds

TOOLCHAIN ?= riscv32-unknown-elf-

# Default optimization level (can be overridden)
OPT_LEVEL ?= -O3

# Base flags (common to all builds)
BASE_FLAGS = -Wall -nostdlib -mabi=ilp32 -march=rv32imzicsr -fno-builtin

# Combined flags
CFLAGS = $(BASE_FLAGS) $(OPT_LEVEL)


# Default build (O3)
build: clean main.bin

# Build with no optimization (for performance comparison)
build-O0:
	$(MAKE) build OPT_LEVEL="-O0"
	mv main.bin main_O0.bin
	mv main.elf main_O0.elf
	@echo "Built with -O0 -> main_O0.bin"

# Build with heavy optimization (for performance comparison)
build-O3:
	$(MAKE) build OPT_LEVEL="-O3"
	mv main.bin main_O3.bin
	mv main.elf main_O3.elf
	@echo "Built with -O3 -> main_O3.bin"

# Build both versions for comparison
build-both: clean
	$(MAKE) build-O0
	$(MAKE) build-O3


main.elf: 
	@echo "Compiling with: $(CFLAGS)"
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

run-O0: main_O0.bin
	make -C $(TOOL_DIR) "FILE_TO_RUN=$(CURDIR)/$<"

run-O3: main_O3.bin
	make -C $(TOOL_DIR) "FILE_TO_RUN=$(CURDIR)/$<"
