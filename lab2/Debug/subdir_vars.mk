################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../LM3S8962.cmd 

CFG_SRCS += \
../app.cfg 

C_SRCS += \
../buttons.c \
../frame_graphics.c \
../main.c \
C:/StellarisWare/boards/ek-lm3s8962/drivers/rit128x96x4.c 

OBJS += \
./buttons.obj \
./frame_graphics.obj \
./main.obj \
./rit128x96x4.obj 

C_DEPS += \
./buttons.pp \
./frame_graphics.pp \
./main.pp \
./rit128x96x4.pp 

GEN_MISC_DIRS += \
./configPkg/ 

GEN_CMDS += \
./configPkg/linker.cmd 

GEN_OPTS += \
./configPkg/compiler.opt 

GEN_FILES += \
./configPkg/linker.cmd \
./configPkg/compiler.opt 

GEN_FILES__QUOTED += \
"configPkg\linker.cmd" \
"configPkg\compiler.opt" 

GEN_MISC_DIRS__QUOTED += \
"configPkg\" 

C_DEPS__QUOTED += \
"buttons.pp" \
"frame_graphics.pp" \
"main.pp" \
"rit128x96x4.pp" 

OBJS__QUOTED += \
"buttons.obj" \
"frame_graphics.obj" \
"main.obj" \
"rit128x96x4.obj" 

GEN_CMDS__FLAG += \
-l"./configPkg/linker.cmd" 

GEN_OPTS__FLAG += \
--cmd_file="./configPkg/compiler.opt" 

C_SRCS__QUOTED += \
"../buttons.c" \
"../frame_graphics.c" \
"../main.c" \
"C:/StellarisWare/boards/ek-lm3s8962/drivers/rit128x96x4.c" 


