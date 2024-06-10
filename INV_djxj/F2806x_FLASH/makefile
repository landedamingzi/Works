################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := D:/ti/ccs1271/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"D:/ti/controlSUITE/device_support/f2806x/v100/F2806x_headers/cmd/F2806x_Headers_nonBIOS.cmd" \
"./F2806x_CodeStartBranch.obj" \
"./F2806x_GlobalVariableDefs.obj" \
"./FlashingLeds-Main.obj" \
"./asm/ADCDRV_1ch.obj" \
"./asm/F2806x_usDelay.obj" \
"./asm/FlashingLeds-DPL-ISR.obj" \
"./asm/PWMDRV_1ch.obj" \
"./asm/PWMDRV_ComplPairDB.obj" \
"./source/ADC_SOC_Cnf.obj" \
"./source/FlashingLeds-DevInit_F2806x.obj" \
"./source/PWM_1ch_Cnf.obj" \
"./source/PWM_ComplPairDB_Cnf.obj" \
"./source/SPLL_1ph_IQ.obj" \
"./source/SPLL_1ph_SOGI_IQ.obj" \
"./source/SciCommsGui.obj" \
"../F28069_FLASH_FlashingLeds.CMD" \
"../IQmath_fpu32.lib" \
"../Solar_Lib_IQ.lib" \
$(GEN_CMDS__FLAG) \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include asm/subdir_vars.mk
-include source/subdir_vars.mk
-include subdir_rules.mk
-include asm/subdir_rules.mk
-include source/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
FlashingLeds.out 

EXE_OUTPUTS__QUOTED += \
"FlashingLeds.out" 


# All Target
all: FlashingLeds.out

# Tool invocations
FlashingLeds.out: $(OBJS) $(CMD_SRCS) $(CMD_UPPER_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: C2000 Linker'
	"D:/ti/ccs1271/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --advice:performance=all --define=_DEBUG --define=LARGE_MODEL --define=FLASH -g --diag_warning=225 --abi=coffabi -z -m"FlashingLeds.map" --stack_size=0x380 --warn_sections -i"D:/ti/ccs1271/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/lib" -i"D:/ti/ccs1271/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" -i"D:/Desktop/CCS_WorkSpace/SPWM_djxj" --reread_libs --xml_link_info="FlashingLeds_linkInfo.xml" --rom_model -o "FlashingLeds.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)
	-$(RM) "F2806x_CodeStartBranch.obj" "F2806x_GlobalVariableDefs.obj" "FlashingLeds-Main.obj" "asm\ADCDRV_1ch.obj" "asm\F2806x_usDelay.obj" "asm\FlashingLeds-DPL-ISR.obj" "asm\PWMDRV_1ch.obj" "asm\PWMDRV_ComplPairDB.obj" "source\ADC_SOC_Cnf.obj" "source\FlashingLeds-DevInit_F2806x.obj" "source\PWM_1ch_Cnf.obj" "source\PWM_ComplPairDB_Cnf.obj" "source\SPLL_1ph_IQ.obj" "source\SPLL_1ph_SOGI_IQ.obj" "source\SciCommsGui.obj" 
	-$(RM) "F2806x_GlobalVariableDefs.d" "FlashingLeds-Main.d" "source\ADC_SOC_Cnf.d" "source\FlashingLeds-DevInit_F2806x.d" "source\PWM_1ch_Cnf.d" "source\PWM_ComplPairDB_Cnf.d" "source\SPLL_1ph_IQ.d" "source\SPLL_1ph_SOGI_IQ.d" "source\SciCommsGui.d" 
	-$(RM) "F2806x_CodeStartBranch.d" "asm\ADCDRV_1ch.d" "asm\F2806x_usDelay.d" "asm\FlashingLeds-DPL-ISR.d" "asm\PWMDRV_1ch.d" "asm\PWMDRV_ComplPairDB.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

