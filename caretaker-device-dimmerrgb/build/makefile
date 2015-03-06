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
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
dimmerrgb.lss \

FLASH_IMAGE += \
dimmerrgb.hex \

EEPROM_IMAGE += \
dimmerrgb.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: dimmerrgb.elf secondary-outputs

# Tool invocations
dimmerrgb.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C++ Linker'
	avr-g++ -Wl,-Map,dimmerrgb.map,--cref -mrelax -Wl,--gc-sections -Wl,--gc-sections -L"/home/grappendorf/workspace-caretaker/caretaker-devices/caretaker-device/build" -L"/home/grappendorf/workspace-arduino/arduino-framework/atmega328p_16000000" -mmcu=atmega328p -o "dimmerrgb.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

dimmerrgb.lss: dimmerrgb.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S dimmerrgb.elf  >"dimmerrgb.lss"
	@echo 'Finished building: $@'
	@echo ' '

dimmerrgb.hex: dimmerrgb.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex dimmerrgb.elf  "dimmerrgb.hex"
	@echo 'Finished building: $@'
	@echo ' '

dimmerrgb.eep: dimmerrgb.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex dimmerrgb.elf  "dimmerrgb.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: dimmerrgb.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=berkeley -t dimmerrgb.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C_UPPER_DEPS)$(S_DEPS)$(LSS)$(C_DEPS)$(FLASH_IMAGE)$(CC_DEPS)$(ELFS)$(C++_DEPS)$(OBJS)$(CXX_DEPS)$(ASM_DEPS)$(EEPROM_IMAGE)$(SIZEDUMMY)$(S_UPPER_DEPS)$(CPP_DEPS) dimmerrgb.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
