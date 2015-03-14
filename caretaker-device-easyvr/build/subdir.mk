################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../easyvr.cpp 

OBJS += \
./easyvr.o 

CPP_DEPS += \
./easyvr.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"/home/grappendorf/workspace-arduino/arduino-framework" -I"/home/grappendorf/workspace-caretaker/caretaker-devices/caretaker-device" -Wall -Os -ffunction-sections -fdata-sections -fno-exceptions -ffunction-sections -fdata-sections -mmcu=atmega168 -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


