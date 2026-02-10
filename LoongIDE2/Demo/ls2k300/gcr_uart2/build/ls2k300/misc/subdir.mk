#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../ls2k300/misc/fls.c \
../ls2k300/misc/memory_man.c \
../ls2k300/misc/newlibc_glue.c \
../ls2k300/misc/print_hex.c

OBJS += \
./ls2k300/misc/fls.o \
./ls2k300/misc/memory_man.o \
./ls2k300/misc/newlibc_glue.o \
./ls2k300/misc/print_hex.o

C_DEPS += \
./ls2k300/misc/fls.d \
./ls2k300/misc/memory_man.d \
./ls2k300/misc/newlibc_glue.d \
./ls2k300/misc/print_hex.d

# Each subdirectory must supply rules for building sources it contributes
ls2k300/misc/%.o: ../ls2k300/misc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: LoongArch64 ELF C Compiler'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-gcc.exe -mabi=lp64d -march=loongarch64 -G0 -DLIB_BSP -DLIB_LWIP -DLS2K300 -DOS_FREERTOS  -O0 -fno-builtin -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../include" -I"../FreeRTOS/osal" -I"../FreeRTOS/include" -I"../FreeRTOS/port" -I"$(GCC_SPECS)/include" -I"$(GCC_BASE)/ls2k-share/lwIP-2.1.3" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

