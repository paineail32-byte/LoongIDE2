#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../BareMetal/PesudoOS/pesudoos.c \
../BareMetal/PesudoOS/pesudo_event.c \
../BareMetal/PesudoOS/pesudo_mq.c \
../BareMetal/PesudoOS/pesudo_mutex.c \
../BareMetal/PesudoOS/pesudo_sem.c \
../BareMetal/PesudoOS/pesudo_task.c \
../BareMetal/PesudoOS/pesudo_timer.c

OBJS += \
./BareMetal/PesudoOS/pesudoos.o \
./BareMetal/PesudoOS/pesudo_event.o \
./BareMetal/PesudoOS/pesudo_mq.o \
./BareMetal/PesudoOS/pesudo_mutex.o \
./BareMetal/PesudoOS/pesudo_sem.o \
./BareMetal/PesudoOS/pesudo_task.o \
./BareMetal/PesudoOS/pesudo_timer.o

C_DEPS += \
./BareMetal/PesudoOS/pesudoos.d \
./BareMetal/PesudoOS/pesudo_event.d \
./BareMetal/PesudoOS/pesudo_mq.d \
./BareMetal/PesudoOS/pesudo_mutex.d \
./BareMetal/PesudoOS/pesudo_sem.d \
./BareMetal/PesudoOS/pesudo_task.d \
./BareMetal/PesudoOS/pesudo_timer.d

# Each subdirectory must supply rules for building sources it contributes
BareMetal/PesudoOS/%.o: ../BareMetal/PesudoOS/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: LoongArch64 ELF C Compiler'
	D:/LoongIDE2/la64-tool/bin/loongarch64-newlib-elf-gcc.exe -mabi=lp64d -march=loongarch64 -G0 -DLIB_BSP -DLS2K300 -DOS_PESUDO  -O0 -fno-builtin -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../include" -I"../BareMetal/osal" -I"../BareMetal/PesudoOS" -I"../ls2k300/include" -I"../ls2k300/drivers/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

