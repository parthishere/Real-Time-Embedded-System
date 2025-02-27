################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/%.obj: ../FreeRTOS/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/home/parth/ti/ccs1260/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="/home/parth/workspace_v12/sequencer" --include_path="/home/parth/workspace_v12/sequencer/driverlib" --include_path="/home/parth/workspace_v12/sequencer" --include_path="/home/parth/workspace_v12/sequencer/FreeRTOS/include" --include_path="/home/parth/workspace_v12/sequencer/FreeRTOS/portable/GCC/ARM_CM4F" --include_path="/home/parth/ti/ccs1260/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs="ccs" --define=DEBUG --define=TARGET_IS_TM4C129_RA1 --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="FreeRTOS/$(basename $(<F)).d_raw" --obj_directory="FreeRTOS" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '


