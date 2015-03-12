################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
configPkg/linker.cmd: ../app.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"c:/ti/xdctools_3_30_03_47_core/xs" --xdcpath="C:/ti/bios_6_41_00_26/packages;c:/ti/ccsv6/ccs_base;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M3 -p ti.platforms.stellaris:LM3S8962 -r release -c "c:/ti/ccsv6/tools/compiler/arm_5.1.8" --compileOptions "-g --optimize_with_debug" "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/compiler.opt: | configPkg/linker.cmd
configPkg/: | configPkg/linker.cmd

buttons.obj: ../buttons.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.8/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.8/include" --include_path="c:/StellarisWare" --include_path="c:/StellarisWare/boards/ek-lm3s8962" -g --gcc --define="ccs" --define=PART_LM3S8962 --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="buttons.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

frame_graphics.obj: ../frame_graphics.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.8/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.8/include" --include_path="c:/StellarisWare" --include_path="c:/StellarisWare/boards/ek-lm3s8962" -g --gcc --define="ccs" --define=PART_LM3S8962 --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="frame_graphics.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.8/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.8/include" --include_path="c:/StellarisWare" --include_path="c:/StellarisWare/boards/ek-lm3s8962" -g --gcc --define="ccs" --define=PART_LM3S8962 --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

rit128x96x4.obj: C:/StellarisWare/boards/ek-lm3s8962/drivers/rit128x96x4.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.8/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.8/include" --include_path="c:/StellarisWare" --include_path="c:/StellarisWare/boards/ek-lm3s8962" -g --gcc --define="ccs" --define=PART_LM3S8962 --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="rit128x96x4.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


