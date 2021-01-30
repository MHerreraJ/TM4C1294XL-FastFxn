################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Peripherals/%.obj: ../Peripherals/%.cpp $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1011/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/ti/tivaware_c_series_2_1_4_178/driverlib" --include_path="D:/manue/Proyectos/CodeComposerStudio/TM4C1294/PeripheralsTestREG" --include_path="C:/ti/ccs1011/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/include" --include_path="C:/ti/tivaware_c_series_2_1_4_178/inc" --define=ccs="ccs" --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="Peripherals/$(basename $(<F)).d_raw" --obj_directory="Peripherals" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


