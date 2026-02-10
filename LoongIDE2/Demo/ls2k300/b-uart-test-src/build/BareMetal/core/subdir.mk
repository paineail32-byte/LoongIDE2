#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
ASM_SRCS += \
../BareMetal/core/cache.S \
../BareMetal/core/err_s.S \
../BareMetal/core/irq_s.S \
../BareMetal/core/start.S

C_SRCS += \
../BareMetal/core/bsp_start.c \
../BareMetal/core/err_c.c \
../BareMetal/core/exception.c \
../BareMetal/core/irq.c \
../BareMetal/core/tick.c

STARTO += ./BareMetal/core/start.o

OBJS += \
./BareMetal/core/bsp_start.o \
./BareMetal/core/cache.o \
./BareMetal/core/err_c.o \
./BareMetal/core/err_s.o \
./BareMetal/core/exception.o \
./BareMetal/core/irq.o \
./BareMetal/core/irq_s.o \
./BareMetal/core/tick.o

ASM_DEPS += \
./BareMetal/core/cache.d \
./BareMetal/core/err_s.d \
./BareMetal/core/irq_s.d \
./BareMetal/core/start.d

C_DEPS += \
./BareMetal/core/bsp_start.d \
./BareMetal/core/err_c.d \
./BareMetal/core/exception.d \
./BareMetal/core/irq.d \
./BareMetal/core/tick.d

# Each subdirectory must supply rules for building sources it contributes
BareMetal/core/%.o: ../BareMetal/core/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: LoongArch64 ELF C Compiler'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-gcc.exe -mabi=lp64d -march=loongarch64 -G0 -DLS2K300 -DOS_PESUDO  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../ls2k300/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

BareMetal/core/%.o: ../BareMetal/core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: LoongArch64 ELF C Compiler'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-gcc.exe -mabi=lp64d -march=loongarch64 -G0 -DLIB_BSP -DLS2K300 -DOS_PESUDO  -O0 -fno-builtin -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../include" -I"../BareMetal/osal" -I"../BareMetal/PesudoOS" -I"../ls2k300/include" -I"../ls2k300/drivers/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

