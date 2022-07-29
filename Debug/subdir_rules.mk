################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-1842988479: ../empty.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs1031/ccs/utils/sysconfig_1.8.1/sysconfig_cli.bat" -s "C:/ti/simplelink_msp432e4_sdk_4_20_00_12/.metadata/product.json" --script "K:/OneDrive/Projects/Chione/software/always-on/MSP_EXP432E401Y_freertos_ccs/empty.syscfg" -o "syscfg" --compiler ccs
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/ti_drivers_config.c: build-1842988479 ../empty.syscfg
syscfg/ti_drivers_config.h: build-1842988479
syscfg/ti_utils_build_linker.cmd.exp: build-1842988479
syscfg/syscfg_c.rov.xs: build-1842988479
syscfg/: build-1842988479

syscfg/%.obj: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --opt_for_speed=5 --fp_mode=relaxed --include_path="C:/FreeRTOSv202012.00/FreeRTOS/Source/include" --include_path="K:/OneDrive/Projects/Chione/software/always-on/MSP_EXP432E401Y_freertos_ccs/inc" --include_path="K:/OneDrive/Projects/Chione/software/always-on/MSP_EXP432E401Y_freertos_ccs" --include_path="K:/OneDrive/Projects/Chione/software/always-on/MSP_EXP432E401Y_freertos_ccs/Debug" --include_path="C:/ti/simplelink_msp432e4_sdk_4_20_00_12/source" --include_path="C:/ti/simplelink_msp432e4_sdk_4_20_00_12/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432e4_sdk_4_20_00_12/source/ti/posix/ccs" --include_path="C:/FreeRTOSv202012.00/FreeRTOS/Source/include" --include_path="C:/FreeRTOSv202012.00/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="K:/OneDrive/Projects/Chione/software/always-on/freertos_builds_MSP_EXP432E401Y_release_ccs" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/include" --include_path="K:/OneDrive/Projects/Chione/software/always-on/free-rtos-plus/FreeRTOS-Plus-TCP/portable/Compiler/GCC" --include_path="K:/OneDrive/Projects/Chione/software/always-on/free-rtos-plus/FreeRTOS-Plus-TCP/include" --include_path="K:/OneDrive/Projects/Chione/software/always-on/free-rtos-plus/FreeRTOS-Plus-CLI" --include_path="K:/OneDrive/Projects/Chione/software/common/unix-timestamp/src" --define=__MSP432E401Y__ -g --c99 --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --gen_data_subsections=on --ramfunc=off --sat_reassoc=on --preproc_with_compile --preproc_dependency="syscfg/$(basename $(<F)).d_raw" --include_path="K:/OneDrive/Projects/Chione/software/always-on/MSP_EXP432E401Y_freertos_ccs/Debug/syscfg" --obj_directory="syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


