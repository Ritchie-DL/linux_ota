cmake_minimum_required(VERSION 3.10)
string(REGEX REPLACE "(.*)/${CMAKE_CURRENT_LIST_DIR}$" "\\1" ABOVE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR})
string(REGEX REPLACE ".*/(.*)" "\\1" CURRENT_FOLDER ${ABOVE_FOLDER})

SET(target_bin ${CURRENT_FOLDER}.bin)

project(${target_bin})

option(CROSS_COMPILE "Cross-compiling" ON)

SET(CROSS_GCC_NAME              arm-rockchip830-linux-uclibcgnueabihf)

if (CMAKE_HOST_UNIX)
    STRING(REGEX MATCH "/home/[0-9A-Za-z]+/" OS_DIR_PREFIX ${CMAKE_CURRENT_LIST_DIR})
    SET(WK_DIR_PREFIX           ${OS_DIR_PREFIX}/workstation)
    STRING(TIMESTAMP            COMPILE_TIME %Y%m%d%H%M%S)
    STRING(TIMESTAMP            COMPILE_DATE %Y%m%d)
    SET(OPT_TOOL_DIR    	    /opt/toolchain/${CROSS_GCC_NAME})
    SET(TOOLCHAIN_DIR   	    ${OPT_TOOL_DIR}/bin)
else()
    SET(OS_DIR_PREFIX           "V:")
    SET(WK_DIR_PREFIX           ${OS_DIR_PREFIX})
    SET(CROSS_COMPILE           OFF)
endif()

SET(ARM_TEST_DIR 		            ${WK_DIR_PREFIX}/arm-test)

if (CROSS_COMPILE STREQUAL ON)
    SET(CROSS_PREFIX 		  ${TOOLCHAIN_DIR}/${CROSS_GCC_NAME}-)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ARM_TEST_DIR}/fs/rknfs/opt/app)
endif ()

SET(CMAKE_CXX_COMPILER 				${CROSS_PREFIX}g++)
SET(CMAKE_C_COMPILER 				${CROSS_PREFIX}gcc)

ADD_COMPILE_DEFINITIONS(_GNU_SOURCE)

INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/ab_meta/include
        ${PROJECT_SOURCE_DIR}/flash/include
        ${PROJECT_SOURCE_DIR}/ota/include
        ${PROJECT_SOURCE_DIR}/upgrade/include
        ${PROJECT_SOURCE_DIR}/test/app_log/include
        ${PROJECT_SOURCE_DIR}/test/eventhub/include
        ${PROJECT_SOURCE_DIR}/test/openssl/include
        ${PROJECT_SOURCE_DIR}/test/utils/include
)

LINK_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/test/eventhub/lib
        ${PROJECT_SOURCE_DIR}/test/openssl/lib
)

FILE(GLOB_RECURSE SRC_LISTS
        ${PROJECT_SOURCE_DIR}/ota/src/*.c
        ${PROJECT_SOURCE_DIR}/flash/src/*.c
        ${PROJECT_SOURCE_DIR}/ab_meta/src/*.c
        ${PROJECT_SOURCE_DIR}/upgrade/src/*.c
        ${PROJECT_SOURCE_DIR}/test/app_log/src/*.c
        ${PROJECT_SOURCE_DIR}/test/utils/src/*.c
        ${PROJECT_SOURCE_DIR}/test/main.c
)

ADD_EXECUTABLE(${target_bin} ${SRC_LISTS})

TARGET_LINK_LIBRARIES(${target_bin}
        eventhub
        ssl
        crypto
)
