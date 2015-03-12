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
../kiss_fft.c \
../main.c \
../network.c \
C:/StellarisWare/boards/ek-lm3s8962/drivers/rit128x96x4.c \
C:/StellarisWare/utils/ustdlib.c 

OBJS += \
./buttons.obj \
./frame_graphics.obj \
./kiss_fft.obj \
./main.obj \
./network.obj \
./rit128x96x4.obj \
./ustdlib.obj 

C_DEPS += \
./buttons.pp \
./frame_graphics.pp \
./kiss_fft.pp \
./main.pp \
./network.pp \
./rit128x96x4.pp \
./ustdlib.pp 

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
"kiss_fft.pp" \
"main.pp" \
"network.pp" \
"rit128x96x4.pp" \
"ustdlib.pp" 

OBJS__QUOTED += \
"buttons.obj" \
"frame_graphics.obj" \
"kiss_fft.obj" \
"main.obj" \
"network.obj" \
"rit128x96x4.obj" \
"ustdlib.obj" 

GEN_CMDS__FLAG += \
-l"./configPkg/linker.cmd" 

GEN_OPTS__FLAG += \
--cmd_file="./configPkg/compiler.opt" 

C_SRCS__QUOTED += \
"../buttons.c" \
"../frame_graphics.c" \
"../kiss_fft.c" \
"../main.c" \
"../network.c" \
"C:/StellarisWare/boards/ek-lm3s8962/drivers/rit128x96x4.c" \
"C:/StellarisWare/utils/ustdlib.c" 


