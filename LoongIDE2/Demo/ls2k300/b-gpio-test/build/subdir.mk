#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../main.c \
../gpio_int_test.c \
../ls2k301_board_gpio_test.c

OBJS += \
./main.o \
./gpio_int_test.o \
./ls2k301_board_gpio_test.o

C_DEPS += \
./main.d \
./gpio_int_test.d \
./ls2k301_board_gpio_test.d

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: LoongArch64 ELF C Compiler'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-gcc.exe -mabi=lp64d -march=loongarch64 -G0 -DLIB_BSP -DLS2K300 -DOS_PESUDO  -O0 -fno-builtin -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../include" -I"../BareMetal/osal" -I"../BareMetal/PesudoOS" -I"$(GCC_SPECS)/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

