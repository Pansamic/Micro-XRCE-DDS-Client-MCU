#############################################################################################################################
# file:  CMakeLists.txt
# brief: Template "CMakeLists.txt" for building of executables and static libraries.
#
# usage: Edit "VARIABLES"-section to suit project requirements.
#        For debug build:
#          cmake -DCMAKE_TOOLCHAIN_FILE=cubeide-gcc.cmake  -S ./ -B Debug -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
#          make -C Debug VERBOSE=1
#        For release build:
#          cmake -DCMAKE_TOOLCHAIN_FILE=cubeide-gcc.cmake  -S ./ -B Release -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
#          make -C Release VERBOSE=1
#############################################################################################################################
cmake_minimum_required(VERSION 3.20)

###################### CONSTANTS ######################################
set (PROJECT_TYPE_EXECUTABLE          "exe")
set (PROJECT_TYPE_STATIC_LIBRARY      "static-lib")
set (MCPU_CORTEX_M0				      "-mcpu=cortex-m0")
set (MCPU_CORTEX_M0PLUS				  "-mcpu=cortex-m0plus")
set (MCPU_CORTEX_M3				      "-mcpu=cortex-m3")
set (MCPU_CORTEX_M4				      "-mcpu=cortex-m4")
set (MCPU_CORTEX_M7				      "-mcpu=cortex-m7")
set (MCPU_CORTEX_M33				  "-mcpu=cortex-m33")
set (MFPU_FPV4_SP_D16                 "-mfpu=fpv4-sp-d16")
set (MFPU_FPV5_D16                    "-mfpu=fpv5-d16")
set (RUNTIME_LIBRARY_REDUCED_C        "--specs=nano.specs")
set (RUNTIME_LIBRARY_STD_C            "")
set (RUNTIME_LIBRARY_SYSCALLS_MINIMAL "--specs=nosys.specs")
set (RUNTIME_LIBRARY_SYSCALLS_NONE    "")
set (MFLOAT_ABI_SOFTWARE              "-mfloat-abi=soft")
set (MFLOAT_ABI_HARDWARE              "-mfloat-abi=hard")
set (MFLOAT_ABI_MIX                   "-mfloat-abi=softfp")
#######################################################################

###################### VARIABLES ######################################
set (PROJECT_NAME             "Micro-XRCE-DDS-Client-MCU")
set (PROJECT_TYPE             "exe")
set (LINKER_SCRIPT            "../STM32F407ZGTX_FLASH.ld")
set (MCPU                     ${MCPU_CORTEX_M4})
set (MFPU                     ${MFPU_FPV4_SP_D16})
set (MFLOAT_ABI               ${MFLOAT_ABI_HARDWARE})
set (RUNTIME_LIBRARY          ${RUNTIME_LIBRARY_REDUCED_C})
set (RUNTIME_LIBRARY_SYSCALLS ${RUNTIME_LIBRARY_SYSCALLS_MINIMAL})


file (GLOB PROJECT_SOURCES
	Core/Src/*.c
	Core/Startup/*.s
	Drivers/STM32F4xx_HAL_Driver/Src/*.c
	Middlewares/Third_Party/cRingbuf/cRingbuf.c
	Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c
	Middlewares/ST/STM32_USB_Device_Library/Core/Src/*.c
	Middlewares/Third_Party/FreeRTOS/Source/*.c
	Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
	Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
	Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c
	USB_DEVICE/App/*.c
	USB_DEVICE/Target/usbd_conf.c
	)

set (PROJECT_DEFINES
	# LIST COMPILER DEFINITIONS HERE
	STM32F407xx
	USE_HAL_DRIVER
	__rtems__
    )

set (PROJECT_INCLUDES
	# LIST INCLUDE DIRECTORIES HERE
	Core/Inc
	Drivers/CMSIS/Include
	Drivers/CMSIS/DSP/Include
	Drivers/CMSIS/Device/ST/STM32F4xx/Include
	Drivers/STM32F4xx_HAL_Driver/Inc
	Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
	Middlewares/ST/STM32_USB_Device_Library/Core/Inc
	Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS
	Middlewares/Third_Party/FreeRTOS/Source/include
	Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
	Middlewares/Third_Party/cRingbuf
	USB_DEVICE/App
	USB_DEVICE/Target
    )

############ MODIFY ACCORDING TO REQUIREMENTS) ########################

#######################################################################
set(CMAKE_TOOLCHAIN_FILE toolchains/gcc-arm-none-eabi.cmake)
################## PROJECT SETUP ######################################
project(${PROJECT_NAME})
enable_language(ASM)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

set (CMAKE_EXECUTABLE_SUFFIX ".elf")
set (CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set (CMAKE_C_FLAGS "${MCPU} -std=gnu11 ${MFPU} ${MFLOAT_ABI} ${RUNTIME_LIBRARY} -mthumb -Wall -Og -g3 -ggdb" CACHE INTERNAL "" FORCE)
set (CMAKE_EXE_LINKER_FLAGS "-T${LINKER_SCRIPT} ${RUNTIME_LIBRARY_SYSCALLS} -Wl,-Map=${PROJECT_NAME}.map -Wl,--gc-sections -static -Wl,--start-group -Wl,--end-group")
set (CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp")

include(ExternalProject)

# Set the URL of the Git repository
set(UXRCE_URL "https://github.com/eProsima/Micro-XRCE-DDS-Client.git")

# Set the destination directory for the cloned repository
set(EXTERNAL_REPO_DIR "${CMAKE_BINARY_DIR}/external")

# Set CMake configuration parameters
set(UXRCE_CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=Debug
	-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
	-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
	-DUCLIENT_PIC=OFF
	-DUCLIENT_PROFILE_DISCOVERY=OFF
	-DUCLIENT_PROFILE_UDP=OFF
	-DUCLIENT_PROFILE_TCP=OFF
	-DUCLIENT_PROFILE_SERIAL=OFF
    # Add more configuration parameters as needed
)

# Use ExternalProject_Add to clone the repository and configure the project
ExternalProject_Add(
    micro-xrce-dds-client
    PREFIX ${EXTERNAL_REPO_DIR}
    GIT_REPOSITORY ${UXRCE_URL}
    TIMEOUT 1000
    CMAKE_ARGS ${UXRCE_CMAKE_ARGS}
)

# Specify the target to depend on the external project
add_dependencies(${PROJECT_NAME} micro-xrce-dds-client)

target_link_libraries(${PROJECT_NAME}
	${CMAKE_BINARY_DIR}/install/lib/libmicroxrcedds_client.a
	${CMAKE_BINARY_DIR}/install/lib/libmicrocdr.a)
target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_BINARY_DIR}/install/include
)

add_compile_definitions(${PROJECT_DEFINES})
include_directories(${PROJECT_INCLUDES})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>)

