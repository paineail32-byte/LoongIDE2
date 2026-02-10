#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../modbus/src/mb.c \
../modbus/src/mb_master.c \
../modbus/src/mb_slave.c \
../modbus/src/mb_util.c

OBJS += \
./modbus/src/mb.o \
./modbus/src/mb_master.o \
./modbus/src/mb_slave.o \
./modbus/src/mb_util.o

C_DEPS += \
./modbus/src/mb.d \
./modbus/src/mb_master.d \
./modbus/src/mb_slave.d \
./modbus/src/mb_util.d

# Each subdirectory must supply rules for building sources it contributes
modbus/src/%.o: ../modbus/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: LoongArch64 ELF C Compiler'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-gcc.exe -mabi=lp64d -march=loongarch64 -G0 -DLIB_BSP -DLIB_LWIP -DLS2K300 -DOS_FREERTOS  -O0 -fno-builtin -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../include" -I"../FreeRTOS/osal" -I"../FreeRTOS/include" -I"../FreeRTOS/port" -I"$(GCC_SPECS)/include" -I"$(GCC_BASE)/ls2k-share/lwIP-2.1.3" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

