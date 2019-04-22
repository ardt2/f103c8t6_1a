################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Code/main103c8t6_1.cpp 

OBJS += \
./Code/main103c8t6_1.o 

CPP_DEPS += \
./Code/main103c8t6_1.d 


# Each subdirectory must supply rules for building sources it contributes
Code/%.o: ../Code/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra  -g3 -DNDEBUG -DSTM32F10X_MD -DHSE_VALUE=8000000 -DSTM32F103xB -DHSI_VALUE=8000000 -DARM_MATH_CM3 -I"../System/Include" -I"../System/Include/Cmsis" -I"../System/Include/stm32f1-stdperiph" -std=c++17 -fabi-version=0 -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics -Weffc++ -Wa,-adhlns="$@.lst" -v -fpermissive -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


