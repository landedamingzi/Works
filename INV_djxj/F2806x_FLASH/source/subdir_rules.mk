################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
source/%.obj: ../source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"D:/ti/ccs1271/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="D:/ti/ccs1271/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --include_path="D:/Desktop/CCS_WorkSpace/SPWM_djxj" --include_path="D:/Desktop/CCS_WorkSpace/SPWM_djxj/include" --include_path="D:/ti/controlSUITE/libs/app_libs/solar/v1.2/IQ/include" --include_path="D:/ti/controlSUITE/libs/app_libs/digital_power/f2806x_v3.5/include" --include_path="D:/ti/controlSUITE/device_support/f2806x/v100/F2806x_headers/include" --include_path="D:/ti/controlSUITE/device_support/f2806x/v100/F2806x_common/include" --include_path="D:/ti/controlSUITE/development_kits/~SupportFiles/F2806x_headers" --advice:performance=all --define=_DEBUG --define=LARGE_MODEL --define=FLASH -g --diag_warning=225 --abi=coffabi --preproc_with_compile --preproc_dependency="source/$(basename $(<F)).d_raw" --obj_directory="source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


