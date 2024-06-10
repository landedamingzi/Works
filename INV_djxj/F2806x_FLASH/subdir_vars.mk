################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
D:/ti/controlSUITE/device_support/f2806x/v100/F2806x_headers/cmd/F2806x_Headers_nonBIOS.cmd 

LIB_SRCS += \
../IQmath_fpu32.lib \
../Solar_Lib_IQ.lib 

ASM_SRCS += \
D:/ti/controlSUITE/device_support/f2806x/v100/F2806x_common/source/F2806x_CodeStartBranch.asm 

CMD_UPPER_SRCS += \
../F28069_FLASH_FlashingLeds.CMD 

C_SRCS += \
D:/ti/controlSUITE/device_support/f2806x/v100/F2806x_headers/source/F2806x_GlobalVariableDefs.c \
../FlashingLeds-Main.c 

C_DEPS += \
./F2806x_GlobalVariableDefs.d \
./FlashingLeds-Main.d 

OBJS += \
./F2806x_CodeStartBranch.obj \
./F2806x_GlobalVariableDefs.obj \
./FlashingLeds-Main.obj 

ASM_DEPS += \
./F2806x_CodeStartBranch.d 

OBJS__QUOTED += \
"F2806x_CodeStartBranch.obj" \
"F2806x_GlobalVariableDefs.obj" \
"FlashingLeds-Main.obj" 

C_DEPS__QUOTED += \
"F2806x_GlobalVariableDefs.d" \
"FlashingLeds-Main.d" 

ASM_DEPS__QUOTED += \
"F2806x_CodeStartBranch.d" 

ASM_SRCS__QUOTED += \
"D:/ti/controlSUITE/device_support/f2806x/v100/F2806x_common/source/F2806x_CodeStartBranch.asm" 

C_SRCS__QUOTED += \
"D:/ti/controlSUITE/device_support/f2806x/v100/F2806x_headers/source/F2806x_GlobalVariableDefs.c" \
"../FlashingLeds-Main.c" 


