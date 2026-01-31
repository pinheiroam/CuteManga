# CMake toolchain for Nintendo Switch (devkitA64).
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=path/to/devkitA64.cmake ..
# Requires DEVKITPRO and PORTLIBS in environment.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

if(NOT DEFINED ENV{DEVKITPRO})
  message(FATAL_ERROR "DEVKITPRO not set. Source devkitA64 environment.")
endif()

set(DEVKITPRO $ENV{DEVKITPRO})
set(PORTLIBS $ENV{DEVKITPRO}/portlibs/switch)

set(CMAKE_C_COMPILER "${DEVKITPRO}/devkitA64/bin/aarch64-none-elf-gcc")
set(CMAKE_AR "${DEVKITPRO}/devkitA64/bin/aarch64-none-elf-ar" CACHE FILEPATH "Archiver")
set(CMAKE_RANLIB "${DEVKITPRO}/devkitA64/bin/aarch64-none-elf-ranlib" CACHE FILEPATH "Ranlib")

set(ARCH "-march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIE")
set(CMAKE_C_FLAGS "${ARCH} -D__SWITCH__ -O2 -ffunction-sections" CACHE STRING "C flags")
set(CMAKE_C_FLAGS_RELEASE "-O3" CACHE STRING "C release flags")

set(CMAKE_FIND_ROOT_PATH ${DEVKITPRO}/devkitA64 ${PORTLIBS})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Help find zlib in portlibs
set(CMAKE_PREFIX_PATH ${PORTLIBS})

# Stub for sys/auxv.h and asm/hwcap.h when building unarr for Switch (no Linux headers)
if(DEFINED ENV{UNARR_STUB_INCLUDE})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I$ENV{UNARR_STUB_INCLUDE}")
endif()
