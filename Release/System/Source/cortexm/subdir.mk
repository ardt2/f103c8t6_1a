################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../System/Source/cortexm/_initialize_hardware.c \
../System/Source/cortexm/_reset_hardware.c \
../System/Source/cortexm/exception_handlers.c 

OBJS += \
./System/Source/cortexm/_initialize_hardware.o \
./System/Source/cortexm/_reset_hardware.o \
./System/Source/cortexm/exception_handlers.o 

C_DEPS += \
./System/Source/cortexm/_initialize_hardware.d \
./System/Source/cortexm/_reset_hardware.d \
./System/Source/cortexm/exception_handlers.d 


# Each subdirectory must supply rules for building sources it contributes
System/Source/cortexm/%.o: ../System/Source/cortexm/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra  -g -DNDEBUG -DSTM32F10X_MD -DHSE_VALUE=8000000 -DSTM32F103xB -DHSI_VALUE=8000000 -DARM_MATH_CM3 -I"../System/Include" -I"../System/Include/Cmsis" -I"../System/Include/stm32f1-stdperiph" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


