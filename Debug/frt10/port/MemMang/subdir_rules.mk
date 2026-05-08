################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
frt10/port/MemMang/%.obj: ../frt10/port/MemMang/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="/Users/emilklitgaard/Documents/Code Composer Studio/Exam" --include_path="/Users/emilklitgaard/Documents/Code Composer Studio/Exam/frt10/inc" --include_path="/Users/emilklitgaard/Documents/Code Composer Studio/Exam/frt10/port/TivaM4" --include_path="/Applications/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs="ccs" --define=PART_TM4C123GH6PM -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="frt10/port/MemMang/$(basename $(<F)).d_raw" --obj_directory="frt10/port/MemMang" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


