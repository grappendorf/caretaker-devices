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
remotecontrol.lss \

FLASH_IMAGE += \
remotecontrol.hex \

EEPROM_IMAGE += \
remotecontrol.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: remotecontrol.elf secondary-outputs

# Tool invocations
remotecontrol.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C++ Linker'
	avr-g++ -Wl,-Map,remotecontrol.map,--cref -mrelax -Wl,--gc-sections -Wl,--gc-sections -L"/home/grappendorf/workspace-caretaker/caretaker-devices/caretaker-device/build" -L"/home/grappendorf/workspace-arduino/arduino-framework/atmega328p_16000000" -mmcu=atmega328p -o "remotecontrol.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

remotecontrol.lss: remotecontrol.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S remotecontrol.elf  >"remotecontrol.lss"
	@echo 'Finished building: $@'
	@echo ' '

remotecontrol.hex: remotecontrol.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex remotecontrol.elf  "remotecontrol.hex"
	@echo 'Finished building: $@'
	@echo ' '

remotecontrol.eep: remotecontrol.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex remotecontrol.elf  "remotecontrol.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: remotecontrol.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=berkeley -t remotecontrol.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C_UPPER_DEPS)$(S_DEPS)$(LSS)$(C_DEPS)$(FLASH_IMAGE)$(CC_DEPS)$(ELFS)$(C++_DEPS)$(OBJS)$(CXX_DEPS)$(ASM_DEPS)$(EEPROM_IMAGE)$(SIZEDUMMY)$(S_UPPER_DEPS)$(CPP_DEPS) remotecontrol.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
