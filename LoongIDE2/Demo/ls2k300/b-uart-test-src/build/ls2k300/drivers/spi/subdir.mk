#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../ls2k300/drivers/spi/ls2k_spi_bus.c

OBJS += \
./ls2k300/drivers/spi/ls2k_spi_bus.o

C_DEPS += \
./ls2k300/drivers/spi/ls2k_spi_bus.d

# Each subdirectory must supply rules for building sources it contributes
ls2k300/drivers/spi/%.o: ../ls2k300/drivers/spi/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: LoongArch64 ELF C Compiler'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-gcc.exe -mabi=lp64d -march=loongarch64 -G0 -DLIB_BSP -DLS2K300 -DOS_PESUDO  -O0 -fno-builtin -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../include" -I"../BareMetal/osal" -I"../BareMetal/PesudoOS" -I"../ls2k300/include" -I"../ls2k300/drivers/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

