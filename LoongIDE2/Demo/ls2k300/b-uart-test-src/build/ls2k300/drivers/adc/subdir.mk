#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
ASM_SRCS += \
../ls2k300/drivers/adc/ls2k_adc_fast.S

C_SRCS += \
../ls2k300/drivers/adc/ls2k_adc.c

OBJS += \
./ls2k300/drivers/adc/ls2k_adc.o \
./ls2k300/drivers/adc/ls2k_adc_fast.o

ASM_DEPS += \
./ls2k300/drivers/adc/ls2k_adc_fast.d

C_DEPS += \
./ls2k300/drivers/adc/ls2k_adc.d

# Each subdirectory must supply rules for building sources it contributes
ls2k300/drivers/adc/%.o: ../ls2k300/drivers/adc/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: LoongArch64 ELF C Compiler'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-gcc.exe -mabi=lp64d -march=loongarch64 -G0 -DLS2K300 -DOS_PESUDO  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../ls2k300/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

ls2k300/drivers/adc/%.o: ../ls2k300/drivers/adc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: LoongArch64 ELF C Compiler'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-gcc.exe -mabi=lp64d -march=loongarch64 -G0 -DLIB_BSP -DLS2K300 -DOS_PESUDO  -O0 -fno-builtin -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../include" -I"../BareMetal/osal" -I"../BareMetal/PesudoOS" -I"../ls2k300/include" -I"../ls2k300/drivers/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

