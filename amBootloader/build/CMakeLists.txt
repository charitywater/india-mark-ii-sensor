# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
# FILE:
#     Build/CMakeLists.txt
#
# Top-Level CMake file for AM bootloader build
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~


INCLUDE(CMakeForceCompiler)

# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
message(STATUS, "CMake Version: ${CMAKE_VERSION} ")

# CURRENT DIRECTORY
SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
SET(CMAKE_SOURCE_DIR ${ProjDirPath}/../)

# Build Timestamp
string(TIMESTAMP  BUILD_TIMESTAMP_UTC  %Y-%m-%dT%H:%M:%SZ  UTC)

# Git Branch Name
execute_process(
    COMMAND bash "-c" "/usr/bin/git symbolic-ref -q --short HEAD || /usr/bin/git describe --tags --exact-match"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Git Abbreviated Commit Hash
execute_process(
  COMMAND /usr/bin/git log --max-count=1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Build version header from template
configure_file(
    ${CMAKE_SOURCE_DIR}/build/version-git-info.h.in
    ${CMAKE_SOURCE_DIR}/inc/version-git-info.h
)


# ENABLE ASM
ENABLE_LANGUAGE(ASM)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

# LINK FILES
SET(CMAKE_EXE_LINKER_FLAGS_GENERIC "-T ${ProjDirPath}/../STM32L4R5VITx_FLASH.ld")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${CMAKE_EXE_LINKER_FLAGS_GENERIC}")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${CMAKE_EXE_LINKER_FLAGS_GENERIC}")

# ASM FLAGS
SET(CMAKE_ASM_FLAGS_GENERIC "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fmessage-length=0 -ffunction-sections -Wall -std=gnu99")
SET(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS_RELEASE} ${CMAKE_ASM_FLAGS_GENERIC}")
SET(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} ${CMAKE_ASM_FLAGS_GENERIC}")

# C FLAGS
SET(CMAKE_C_FLAGS_GENERIC "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fmessage-length=0 -ffunction-sections -Wall -std=gnu99")
SET(CMAKE_C_FLAGS_GENERIC "${CMAKE_C_FLAGS_GENERIC} -DSTM32L4R5xx")
SET(CMAKE_C_FLAGS_GENERIC "${CMAKE_C_FLAGS_GENERIC} -DUSE_HAL_DRIVER")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${CMAKE_C_FLAGS_GENERIC}")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${CMAKE_C_FLAGS_GENERIC}")

# LD FLAGS
SET(CMAKE_EXE_LINKER_FLAGS_GENERIC "-Xlinker --gc-sections -specs=nosys.specs -specs=nano.specs")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${CMAKE_EXE_LINKER_FLAGS_GENERIC}")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${CMAKE_EXE_LINKER_FLAGS_GENERIC}")


# INCLUDE_DIRECTORIES
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/inc)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/bootloader)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/flashCommunication)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/logging)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/watchdog)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Inc)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Inc/Legacy)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/drivers/CMSIS/Device/ST/STM32L4xx/Include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/drivers/CMSIS/Include)


# ADD_EXECUTABLE
ADD_EXECUTABLE(am-boot
    "${CMAKE_SOURCE_DIR}/startup/startup_stm32l4r5xx.s"
    "${CMAKE_SOURCE_DIR}/src/main.c"
    "${CMAKE_SOURCE_DIR}/src/stm32l4xx_it.c"
    "${CMAKE_SOURCE_DIR}/src/stm32l4xx_hal_msp.c"
    "${CMAKE_SOURCE_DIR}/src/syscalls.c"
    "${CMAKE_SOURCE_DIR}/src/system_stm32l4xx.c"
    "${CMAKE_SOURCE_DIR}/src/bootloader/bootloader.c"
    "${CMAKE_SOURCE_DIR}/src/bootloader/manufacturing.c"
    "${CMAKE_SOURCE_DIR}/src/bootloader/updateFw.c"
    "${CMAKE_SOURCE_DIR}/src/watchdog/externalWatchdog.c"
    "${CMAKE_SOURCE_DIR}/src/flashCommunication/flashHandler.c"
    "${CMAKE_SOURCE_DIR}/src/flashCommunication/memMapHandler.c"
    "${CMAKE_SOURCE_DIR}/src/flashCommunication/MT29F1.c"
    "${CMAKE_SOURCE_DIR}/src/flashCommunication/spi.c"
    "${CMAKE_SOURCE_DIR}/src/logging/logger.c"
    "${CMAKE_SOURCE_DIR}/src/logging/uart.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c"
)
SET_TARGET_PROPERTIES(am-boot PROPERTIES OUTPUT_NAME "am-boot.elf")

# LIBRARIES
TARGET_LINK_LIBRARIES(am-boot -Wl,--start-group)
TARGET_LINK_LIBRARIES(am-boot m)
TARGET_LINK_LIBRARIES(am-boot -Wl,--end-group)

# MAP FILE
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Xlinker -Map=output.map")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Xlinker -Map=output.map")

# Create hex/bin
ADD_CUSTOM_COMMAND(TARGET am-boot POST_BUILD COMMAND ${CMAKE_OBJCOPY} -Oihex ${ProjDirPath}/am-boot.elf ${ProjDirPath}/am-boot.hex)
ADD_CUSTOM_COMMAND(TARGET am-boot POST_BUILD COMMAND ${CMAKE_OBJCOPY} -Obinary ${ProjDirPath}/am-boot.elf ${ProjDirPath}/am-boot.bin)
