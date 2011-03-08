cmake_minimum_required(VERSION 2.6)

project (QMF-EXPLORER)
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(PROJECT_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

MACRO (inherit_value theSymbol theValue)
    if (NOT DEFINED ${theSymbol})
        set (${theSymbol} ${theValue})
        # message ("Set symbol '${theSymbol}' to value '${theValue}'")
        set (${theSymbol}_inherited = "true")
    endif (NOT DEFINED ${theSymbol})
ENDMACRO (inherit_value)

# Various defines 
inherit_value(QMFE_DEFAULT_URL "localhost")

# Add sub-directories
add_subdirectory(src)

# Installer
set(CPACK_PACKAGE_NAME "qmfexplorer")
set(CPACK_PACKAGE_VENDOR "Red Hat")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "QMF Explorer")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "qmfexplorer-${CPACK_PACKAGE_VERSION}")

INCLUDE(InstallRequiredSystemLibraries)
INCLUDE(CPack)
