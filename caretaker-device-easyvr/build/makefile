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
easyvr.lss \

FLASH_IMAGE += \
easyvr.hex \

EEPROM_IMAGE += \
easyvr.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: easyvr.elf secondary-outputs

# Tool invocations
easyvr.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C++ Linker'
	avr-gcc -Wl,-Map,easyvr.map,--cref -mrelax -Wl,--gc-sections -Wl,--gc-sections -L"/home/grappendorf/workspace-arduino/arduino-framework/atmega168_16000000" -mmcu=atmega168 -o "easyvr.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

easyvr.lss: easyvr.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S easyvr.elf  >"easyvr.lss"
	@echo 'Finished building: $@'
	@echo ' '

easyvr.hex: easyvr.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex easyvr.elf  "easyvr.hex"
	@echo 'Finished building: $@'
	@echo ' '

easyvr.eep: easyvr.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex easyvr.elf  "easyvr.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: easyvr.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=berkeley -t easyvr.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EEPROM_IMAGE)$(FLASH_IMAGE)$(ELFS)$(LSS)$(S_DEPS)$(CXX_DEPS)$(S_UPPER_DEPS)$(C++_DEPS)$(ASM_DEPS)$(CC_DEPS)$(CPP_DEPS)$(C_UPPER_DEPS)$(SIZEDUMMY) easyvr.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
