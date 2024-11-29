################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/scheduler/scheduler.c \
../Core/Src/scheduler/taskQueue.c 

OBJS += \
./Core/Src/scheduler/scheduler.o \
./Core/Src/scheduler/taskQueue.o 

C_DEPS += \
./Core/Src/scheduler/scheduler.d \
./Core/Src/scheduler/taskQueue.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/scheduler/%.o Core/Src/scheduler/%.su: ../Core/Src/scheduler/%.c Core/Src/scheduler/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103x6 -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-scheduler

clean-Core-2f-Src-2f-scheduler:
	-$(RM) ./Core/Src/scheduler/scheduler.d ./Core/Src/scheduler/scheduler.o ./Core/Src/scheduler/scheduler.su ./Core/Src/scheduler/taskQueue.d ./Core/Src/scheduler/taskQueue.o ./Core/Src/scheduler/taskQueue.su

.PHONY: clean-Core-2f-Src-2f-scheduler

