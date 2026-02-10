#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../modbus/port/mb_bsp.c \
../modbus/port/mb_os.c

OBJS += \
./modbus/port/mb_bsp.o \
./modbus/port/mb_os.o

C_DEPS += \
./modbus/port/mb_bsp.d \
./modbus/port/mb_os.d

# Each subdirectory must supply rules for building sources it contributes
modbus/port/%.o: ../modbus/port/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: LoongArch64 ELF C Compiler'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-gcc.exe -mabi=lp64d -march=loongarch64 -G0 -DLIB_BSP -DLIB_LWIP -DLS2K300 -DOS_FREERTOS  -O0 -fno-builtin -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../include" -I"../FreeRTOS/osal" -I"../FreeRTOS/include" -I"../FreeRTOS/port" -I"$(GCC_SPECS)/include" -I"$(GCC_BASE)/ls2k-share/lwIP-2.1.3" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

