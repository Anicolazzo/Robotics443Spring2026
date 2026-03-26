################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccs1281/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1281/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab 8/Lab 8" --include_path="C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab 8" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

AP.obj: C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab\ 8/AP.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccs1281/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1281/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab 8/Lab 8" --include_path="C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab 8" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="AP.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Clock.obj: C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab\ 8/Clock.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccs1281/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1281/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab 8/Lab 8" --include_path="C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab 8" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="Clock.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

GPIO.obj: C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab\ 8/GPIO.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccs1281/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1281/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab 8/Lab 8" --include_path="C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab 8" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="GPIO.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

UART1.obj: C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab\ 8/UART1.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccs1281/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1281/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab 8/Lab 8" --include_path="C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab 8" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="UART1.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

main.obj: C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab\ 8/main.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccs1281/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1281/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab 8/Lab 8" --include_path="C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab 8" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="main.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


