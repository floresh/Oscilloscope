################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := c:/ti/ccsv6/tools/compiler/arm_5.1.8

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./buttons.obj" \
"./frame_graphics.obj" \
"./kiss_fft.obj" \
"./main.obj" \
"./network.obj" \
"./rit128x96x4.obj" \
"./ustdlib.obj" \
"../LM3S8962.cmd" \
-l"libc.a" \
-l"C:\StellarisWare/driverlib/ccs-cm3/Debug/driverlib-cm3.lib" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
Copy\ of\ ece3849b14_lab2_2_hfloreshuerta_ardymek.out \

EXE_OUTPUTS__QUOTED += \
"Copy of ece3849b14_lab2_2_hfloreshuerta_ardymek.out" \

BIN_OUTPUTS += \
Copy\ of\ ece3849b14_lab2_2_hfloreshuerta_ardymek.hex \

BIN_OUTPUTS__QUOTED += \
"Copy of ece3849b14_lab2_2_hfloreshuerta_ardymek.hex" \


# All Target
all: Copy\ of\ ece3849b14_lab2_2_hfloreshuerta_ardymek.out

# Tool invocations
Copy\ of\ ece3849b14_lab2_2_hfloreshuerta_ardymek.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Linker'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.8/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me -g --gcc --define="ccs" --define=PART_LM3S8962 --display_error_number --diag_warning=225 --diag_wrap=off -z -m"Copy of ece3849b14_lab2_2_hfloreshuerta_ardymek.map" --heap_size=0 --stack_size=2048 -i"c:/ti/ccsv6/tools/compiler/arm_5.1.8/lib" -i"c:/ti/ccsv6/tools/compiler/arm_5.1.8/include" --reread_libs --warn_sections --display_error_number --diag_wrap=off --xml_link_info="Copy of ece3849b14_lab2_2_hfloreshuerta_ardymek_linkInfo.xml" --rom_model -o "Copy of ece3849b14_lab2_2_hfloreshuerta_ardymek.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

Copy\ of\ ece3849b14_lab2_2_hfloreshuerta_ardymek.hex: $(EXE_OUTPUTS)
	@echo 'Invoking: ARM Hex Utility'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.8/bin/armhex"  -o "Copy of ece3849b14_lab2_2_hfloreshuerta_ardymek.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(GEN_MISC_FILES__QUOTED)$(EXE_OUTPUTS__QUOTED)$(GEN_FILES__QUOTED)$(BIN_OUTPUTS__QUOTED)$(GEN_OPTS__QUOTED)$(GEN_CMDS__QUOTED)
	-$(RMDIR) $(GEN_MISC_DIRS__QUOTED)
	-$(RM) "buttons.pp" "frame_graphics.pp" "kiss_fft.pp" "main.pp" "network.pp" "rit128x96x4.pp" "ustdlib.pp" 
	-$(RM) "buttons.obj" "frame_graphics.obj" "kiss_fft.obj" "main.obj" "network.obj" "rit128x96x4.obj" "ustdlib.obj" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

