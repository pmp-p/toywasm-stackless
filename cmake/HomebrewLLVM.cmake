if(NOT DEFINED PREFER_BREW_LLVM)
set(PREFER_BREW_LLVM ON)
endif()
if(NOT CMAKE_TOOLCHAIN_FILE AND PREFER_BREW_LLVM)
# Prefer the homebrew version because xcode clang doesn't have detect_leaks
# Note: CMAKE_SYSTEM_NAME is not available yet.
set(BREW_LLVM_HOME /usr/local/opt/llvm@13)
find_program(BREW_CLANG ${BREW_LLVM_HOME}/bin/clang)
if(BREW_CLANG)
set(CMAKE_C_COMPILER ${BREW_CLANG} CACHE FILEPATH "brew llvm clang" FORCE)
find_program(BREW_AR ${BREW_LLVM_HOME}/bin/llvm-ar REQUIRED)
set(CMAKE_AR ${BREW_AR} CACHE FILEPATH "brew llvm ar" FORCE)
set(CMAKE_C_COMPILER_AR ${BREW_AR} CACHE FILEPATH "brew llvm ar" FORCE)
find_program(BREW_RANLIB ${BREW_LLVM_HOME}/bin/llvm-ranlib REQUIRED)
set(CMAKE_RANLIB ${BREW_RANLIB} CACHE FILEPATH "brew llvm ar" FORCE)
set(CMAKE_C_COMPILER_RANLIB ${BREW_RANLIB} CACHE FILEPATH "brew llvm ranlib" FORCE)
endif()
endif()