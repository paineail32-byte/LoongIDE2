#
# Auto-generated file. Do not edit!
#

GCC_SPECS := D:/LoongIDE2/la64-tool/loongarch64-newlib-elf/ls2k300
GCC_BASE := D:/LoongIDE2/la64-tool/loongarch64-newlib-elf
CPU := ls2k300
OS := bare

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include objects.mk
-include subdir.mk
-include ls2k300/misc/subdir.mk
-include src/subdir.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
endif

-include ../makefile.defs

OUT_ELF=b-uart-test.elf
OUT_BIN=b-uart-test.bin
OUT_MAP=b-uart-test.map

LINKCMDS=../ld.script

# Add inputs and outputs from these tool invocations to the build variables

# All Target
all: $(OUT_ELF)

# Tool invocations
$(OUT_ELF): $(STARTO) $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: LoongArch64 ELF C Linker'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-gcc.exe  -mabi=lp64d -march=loongarch64 -G0 -nostartfiles -static -T $(LINKCMDS) -o $(OUT_ELF) $(STARTO) $(OBJS) $(USER_OBJS) $(LIBS) -L"$(GCC_SPECS)/$(OS)" 
	@echo 'Invoking: LoongArch64 ELF C Objcopy'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-objcopy.exe -O binary $(OUT_ELF) $(OUT_BIN)
	@echo 'Invoking: LoongArch64 ELF C Size'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-size.exe $(OUT_ELF)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(STARTO) $(OBJS) $(C_DEPS) $(CPP_DEPS) $(ASM_DEPS) $(EXECUTABLES) $(OUT_ELF) $(OUT_MAP) $(OUT_BIN)
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets


