################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
reflow-oven-arduino.lss \

FLASH_IMAGE += \
reflow-oven-arduino.hex \

EEPROM_IMAGE += \
reflow-oven-arduino.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: reflow-oven-arduino.elf secondary-outputs

# Tool invocations
reflow-oven-arduino.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C++ Linker'
	avr-gcc -Wl,-Map,reflow-oven-arduino.map,--cref -mrelax -Wl,--gc-sections -Wl,--gc-sections -L"/home/grappendorf/workspace-arduino/arduino-framework/atmega328p_16000000" -mmcu=atmega328p -o "reflow-oven-arduino.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

reflow-oven-arduino.lss: reflow-oven-arduino.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S reflow-oven-arduino.elf  >"reflow-oven-arduino.lss"
	@echo 'Finished building: $@'
	@echo ' '

reflow-oven-arduino.hex: reflow-oven-arduino.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex reflow-oven-arduino.elf  "reflow-oven-arduino.hex"
	@echo 'Finished building: $@'
	@echo ' '

reflow-oven-arduino.eep: reflow-oven-arduino.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex reflow-oven-arduino.elf  "reflow-oven-arduino.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: reflow-oven-arduino.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=berkeley -t reflow-oven-arduino.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EEPROM_IMAGE)$(FLASH_IMAGE)$(ELFS)$(LSS)$(S_DEPS)$(CXX_DEPS)$(S_UPPER_DEPS)$(C++_DEPS)$(ASM_DEPS)$(CC_DEPS)$(CPP_DEPS)$(C_UPPER_DEPS)$(SIZEDUMMY) reflow-oven-arduino.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
