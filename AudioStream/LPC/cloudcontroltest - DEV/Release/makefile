################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include example/src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: cloudcontrol_dev.axf

# Tool invocations
cloudcontrol_dev.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"/Users/reed/Documents/LPCXpresso_8.2.2/workspace/lpc_chip_40xx/Release" -L"/Users/reed/Documents/LPCXpresso_8.2.2/workspace/lpc_board_ea_devkit_4088/Release" -Xlinker -Map="cloudcontrol_dev.map" -Xlinker --gc-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -T "cloudcontrol_dev_Release.ld" -o "cloudcontrol_dev.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) cloudcontrol_dev.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "cloudcontrol_dev.axf"; # arm-none-eabi-objcopy -v -O binary "cloudcontrol_dev.axf" "cloudcontrol_dev.bin" ; # checksum -p LPC4088 -d "cloudcontrol_dev.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
