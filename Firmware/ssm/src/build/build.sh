#!/bin/bash

#
# Compile SSM code using cl430 compiler
#

COMPILER_PATH="/ti-cgt-msp430_18.12.3.LTS/bin/"
COMPILER="cl430"
HEX="hex430"
OUTPUT_NAME=ssm

# These are options that are used for BOTH building AND linking.  Please be careful adding here.
GENERIC_OPTIONS=(   -vmspx \
                    --code_model=large \
                    --data_model=large \
                    -O2 \
                    --opt_for_speed=0 \
                    --use_hw_mpy=F5 \
                    --near_data=none \
                    --advice:hw_config="all" \
                    --define=__MSP430FR2676__ \
                    --define=TARGET_IS_MSP430FR2XX_4XX \
                    --define=_INFO_FRWP_ENABLE \
                    --define=__TI_TIME_USES_64 \
                    --define=SSM_BUILD \
                    -g \
                    --gcc \
                    --printf_support=nofloat \
                    --diag_warning=225 \
                    --diag_suppress=10229 \
                    --diag_wrap=off \
                    --display_error_number \
                    --silicon_errata=CPU21 \
                    --silicon_errata=CPU22 \
                    --silicon_errata=CPU40)

# Additional options for building only
BUILD_OPTIONS=( --preproc_with_compile \
                --verbose_diagnostics)

# Include paths for building
BUILD_INCLUDE_PATHS=(   --include_path="/msp430/include" \
                        --include_path="../" \
                        --include_path="../driverlib" \
                        --include_path="../commandLineDriver" \
                        --include_path="../HW/inc" \
                        --include_path="../APP/inc" \
                        --include_path="../uC/inc" \
                        --include_path="../driverlib/MSP430FR2xx_4xx" \
			            --include_path="../driverlib/lis2mdl" \
                        --include_path="../mathlib" \
                        --include_path="../captivate" \
                        --include_path="../captivate/ADVANCED" \
                        --include_path="../captivate/BASE" \
                        --include_path="../captivate/COMM" \
                        --include_path="../captivate_app" \
                        --include_path="../captivate_config" \
                        --include_path="../protocol/inc" \
                        --include_path="/ti-cgt-msp430_18.12.3.LTS/include" \
                        --include_path="../../../shared/asp/inc" \
                        --include_path="../../../shared/nvm/inc" \
                        --include_path="../algo-c-code/calculateWaterVolume" \
                        --include_path="../algo-c-code/clearMagWindowProcess" \
                        --include_path="../algo-c-code/clearPadWindowProcess" \
                        --include_path="../algo-c-code/cliResetStrokeCount" \
                        --include_path="../algo-c-code/computePumpHealth" \
                        --include_path="../algo-c-code/detectStrokes" \
                        --include_path="../algo-c-code/detectTransitions" \
                        --include_path="../algo-c-code/hourlyStrokeCount" \
                        --include_path="../algo-c-code/hourlyWaterVolume" \
                        --include_path="../algo-c-code/getMaxUsageTime" \
                        --include_path="../algo-c-code/initializeMagCalibration" \
                        --include_path="../algo-c-code/initializeStrokeAlgorithm" \
                        --include_path="../algo-c-code/initializeWaterAlgorithm" \
                        --include_path="../algo-c-code/initializeWindows" \
                        --include_path="../algo-c-code/magnetometerCalibration" \
                        --include_path="../algo-c-code/wakeupDataReset" \
                        --include_path="../algo-c-code/waterPadFiltering" \
                        --include_path="../algo-c-code/writeMagSample" \
                        --include_path="../algo-c-code/writePadSample")

# Additional options for linking only
LINK_OPTIONS=(  -z \
                --heap_size=160 \
                --stack_size=512 \
                --cinit_hold_wdt=on \
                --priority \
                --reread_libs \
                --diag_suppress=10229 \
                --diag_wrap=off \
                --display_error_number \
                --warn_sections \
                --xml_link_info="src_linkInfo.xml" \
                --rom_model)

# Include paths for linking
LINK_INCLUDE_PATHS=(  -i"/msp430/include" \
                      -i"/ti-cgt-msp430_18.12.3.LTS/lib" \
                      -i"/ti-cgt-msp430_18.12.3.LTS/include" \
                      -i"/msp430/lib/FR2xx" \
                      -i"/msp430/lib/4xx" \
                      -i"..")

# Libraries that we want to use for linking
LINK_LIBRARIES=(  "../captivate/BASE/libraries/captivate_fr2676_family.lib" \
                  "../driverlib/MSP430FR2xx_4xx/libraries/driverlib.lib" \
                  "../driverlib/MSP430FR2xx_4xx/libraries/driverlib_lcld.lib" \
                  "../driverlib/MSP430FR2xx_4xx/libraries/driverlib_lcsd.lib" \
                  "../driverlib/MSP430FR2xx_4xx/libraries/driverlib_scsd.lib" \
                  "../mathlib/libraries/IQmathLib.lib" \
                  "../mathlib/libraries/IQmathLib_CCS_MPY32_5xx_6xx_CPUX_large_code_large_data.lib" \
                  "../mathlib/libraries/QmathLib.lib" \
                  "../mathlib/libraries/QmathLib_CCS_MPY32_5xx_6xx_CPUX_large_code_large_data.lib"  \
                  -llibmath.a \
                  -lfrwp_init.a \
                  -l"../captivate/BASE/libraries/captivate_fr2676_family.lib" \
                  -l"../driverlib/MSP430FR2xx_4xx/libraries/driverlib.lib" \
                  -l"../mathlib/libraries/QmathLib.lib" \
                  -l"../mathlib/libraries/IQmathLib.lib" \
                  -llibc.a)

# Files that we want to build and then link
# As files are added/changed, this list should be updated
# The *.c at the end of each file is omitted for flexibility in the BASH script
FILES=( "../APP/APP" \
        "../APP/APP_NVM_Cfg" \
        "../main" \
        "../APP/APP_NVM_Custom" \
        "../APP/APP_WTR" \
        "../HW/HW" \
        "../APP/APP_CLI" \
        "../APP/APP_NVM" \
        "../APP/APP_ALGO" \
        "../HW/HW_AM" \
        "../HW/HW_BAT" \
        "../HW/HW_EEP" \
        "../HW/HW_RTC" \
        "../HW/HW_GPIO" \
        "../HW/HW_ENV" \
	    "../HW/HW_MAG" \
        "../captivate/ADVANCED/CAPT_Manager" \
        "../HW/HW_TERM" \
        "../captivate/BASE/CAPT_ISR" \
        "../captivate/COMM/CAPT_Interface" \
        "../captivate/COMM/I2CSlave" \
        "../captivate/COMM/FunctionTimer" \
        "../captivate_app/CAPT_App" \
        "../captivate_app/CAPT_BSP" \
        "../captivate/COMM/UART" \
        "../captivate_config/CAPT_UserConfig" \
        "../commandLineDriver/commandLineDriver" \
        "../driverlib/lis2mdl/lis2mdl_reg" \
        "../uC/uC" \
        "../uC/uC_ADC" \
        "../uC/uC_I2C" \
        "../uC/uC_TIME" \
        "../uC/uC_UART" \
        "../uC/uC_SPI" \
        "../../../shared/asp/am-ssm-spi-protocol" \
        "../../../shared/asp/ssm-spi-protocol" \
        "../algo-c-code/calculateWaterVolume/addToAverage" \
        "../algo-c-code/calculateWaterVolume/calculateWaterVolume" \
        "../algo-c-code/calculateWaterVolume/promotePadStates" \
        "../algo-c-code/calculateWaterVolume/checkWaterCalibration" \
        "../algo-c-code/calculateWaterVolume/detectWaterChange" \
        "../algo-c-code/calculateWaterVolume/waterCalibration" \
        "../algo-c-code/clearMagWindowProcess/clearMagWindowProcess" \
        "../algo-c-code/clearPadWindowProcess/clearPadWindowProcess" \
        "../algo-c-code/cliResetStrokeCount/cliResetStrokeCount" \
        "../algo-c-code/computePumpHealth/computePumpHealth" \
        "../algo-c-code/detectStrokes/detectStrokes" \
        "../algo-c-code/detectTransitions/detectTransitions" \
        "../algo-c-code/hourlyStrokeCount/hourlyStrokeCount" \
        "../algo-c-code/hourlyWaterVolume/hourlyWaterVolume" \
        "../algo-c-code/getMaxUsageTime/getMaxUsageTime" \
        "../algo-c-code/initializeMagCalibration/initializeMagCalibration" \
        "../algo-c-code/initializeStrokeAlgorithm/initializeStrokeAlgorithm" \
        "../algo-c-code/initializeWaterAlgorithm/initializeWaterAlgorithm" \
        "../algo-c-code/initializeWindows/initializeWindows" \
        "../algo-c-code/magnetometerCalibration/magnetometerCalibration" \
	"../algo-c-code/magnetometerCalibration/isPeakValley" \
	"../algo-c-code/magnetometerCalibration/trackRange" \
        "../algo-c-code/wakeupDataReset/wakeupDataReset" \
        "../algo-c-code/waterPadFiltering/waterPadFiltering" \
        "../algo-c-code/writeMagSample/writeMagSample" \
        "../algo-c-code/writePadSample/writePadSample")

# ------------------------------------------------------------------------------------------------------
# Shouldn't need to edit below this line.
# This is where the magic happens.
#
#                        _,-'|
#                     ,-'._  |
#           .||,      |####\ |
#          \.`',/     \####| |
#          = ,. =      |###| |
#          / || \    ,-'\#/,'`.
#            ||     ,'   `,,. `.
#            ,|____,' , ,;' \| |
#           (3|\    _/|/'   _| |
#            ||/,-''  | >-'' _,\\
#            ||'      ==\ ,-'  ,'
#            ||       |  V \ ,|
#            ||       |    |` |
#            ||       |    |   \
#            ||       |    \    \
#            ||       |     |    \
#            ||       |      \_,-'
#            ||       |___,,--")_\
#            ||         |_|   ccc/
#            ||        ccc/
#            ||
# ------------------------------------------------------------------------------------------------------

echo
echo Generating GIT version file
sed "s/@GIT_COMMIT_HASH@/$(/usr/bin/git log --max-count=1 --format=%h)/g" version-git-info.h.in > version-git-info.h
sed -i "s/@GIT_BRANCH_NAME@/$(/usr/bin/git symbolic-ref -q --short HEAD || /usr/bin/git describe --tags --exact-match)/g" version-git-info.h
sed -i "s/@BUILD_TIMESTAMP_UTC@/$(date -u +%Y-%m-%dT%H:%M:%SZ)/g" version-git-info.h
mv version-git-info.h ../APP/inc
echo Finished generating GIT version file


length=${#FILES[@]}

# Build all individual files
for ((i=0;i<$length;i++)); do
    FULL_PATH=${FILES[$i]}
    FOLDER=$(echo $FULL_PATH | sed -r 's:(.*/)(.*$):\1:' ) # Strip down to only folder
    echo Building file: $FULL_PATH.c
    echo Invoking $COMPILER Compiler
    BUILD_COMMAND="$COMPILER_PATH$COMPILER ${GENERIC_OPTIONS[@]} ${BUILD_OPTIONS[@]} ${BUILD_INCLUDE_PATHS[@]} --preproc_dependency=$FULL_PATH.d_raw --obj_directory=$FOLDER $FULL_PATH.c"
    echo $BUILD_COMMAND
    $BUILD_COMMAND
    if [ $? -ne 0 ]
    then
        exit 1
    fi
  echo "Successfully created file"
    echo Finished building: $FULL_PATH.c
    echo
done

# Link files and generate *.out file
echo
echo Building target: $OUTPUT_NAME.out
echo Invoking: $COMPILER Linker
LINK_COMMAND="$COMPILER_PATH$COMPILER ${GENERIC_OPTIONS[@]} ${LINK_OPTIONS[@]} -m$OUTPUT_NAME.map ${LINK_INCLUDE_PATHS[@]} -o $OUTPUT_NAME.out ${FILES[@]/%/.obj} ../lnk_msp430fr2676.cmd ${LINK_LIBRARIES[@]}"
echo $LINK_COMMAND
$LINK_COMMAND
if [ $? -ne 0 ]
then
    exit 1
fi
echo Finished building target: $OUTPUT_NAME.out


echo
echo Creating TI TXT file
HEX_COMMAND="$COMPILER_PATH$HEX --ti_txt -map=$OUTPUT_NAME.map $OUTPUT_NAME.out"
echo $HEX_COMMAND
$HEX_COMMAND
if [ $? -ne 0 ]
then
    exit 1
fi
echo Finished creating TI TXT file.
