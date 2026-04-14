################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-armllvm_3.2.2.LTS/bin/tiarmclang.exe" -c -march=thumbv7em -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mlittle-endian -mthumb -O0 -I"C:/ti/ccs1281/ccs/ccs_base/arm/include" -I"C:/ti/ccs1281/ccs/ccs_base/arm/include/CMSIS" -I"C:/Users/anicolazzo/Documents/GitHub/Robotics443Spring2026/Lab 9/Lab 9" -I"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-armllvm_3.2.2.LTS/include" -Dccs -D__MSP432P401R__ -gdwarf-3 -Wall -Werror=ti-pragmas -Werror=ti-macros -Werror=ti-intrinsics -fno-short-wchar -fcommon -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -std=gnu90 $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


