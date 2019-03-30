################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../System/Source/Cmsis/system_stm32f10x.c \
../System/Source/Cmsis/vectors_stm32f10x.c 

OBJS += \
./System/Source/Cmsis/system_stm32f10x.o \
./System/Source/Cmsis/vectors_stm32f10x.o 

C_DEPS += \
./System/Source/Cmsis/system_stm32f10x.d \
./System/Source/Cmsis/vectors_stm32f10x.d 


# Each subdirectory must supply rules for building sources it contributes
System/Source/Cmsis/%.o: ../System/Source/Cmsis/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DSTM32F10X_MD -DHSE_VALUE=8000000 -DSTM32F103xB -DHSI_VALUE=8000000 -I"../System/Include" -I"../System/Include/Cmsis" -I"../System/Include/stm32f1-stdperiph" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


