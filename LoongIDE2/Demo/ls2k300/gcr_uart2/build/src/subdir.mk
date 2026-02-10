#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../src/bsp_start_hook.c \
../src/ls2k_devices_init_hook.c \
../src/ls2k_devices_register_fs.c

OBJS += \
./src/bsp_start_hook.o \
./src/ls2k_devices_init_hook.o \
./src/ls2k_devices_register_fs.o

C_DEPS += \
./src/bsp_start_hook.d \
./src/ls2k_devices_init_hook.d \
./src/ls2k_devices_register_fs.d

# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: LoongArch64 ELF C Compiler'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-gcc.exe -mabi=lp64d -march=loongarch64 -G0 -DLIB_BSP -DLIB_LWIP -DLS2K300 -DOS_FREERTOS  -O0 -fno-builtin -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../include" -I"../FreeRTOS/osal" -I"../FreeRTOS/include" -I"../FreeRTOS/port" -I"$(GCC_SPECS)/include" -I"$(GCC_BASE)/ls2k-share/lwIP-2.1.3" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

