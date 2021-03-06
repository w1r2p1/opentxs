# Copyright (c) Monetas AG, 2014

cmake_minimum_required(VERSION 2.8.12)
project(opentxs)

#-----------------------------------------------------------------------------
if(NOT ANDROID)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM LAST)
find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND git "describe" OUTPUT_VARIABLE GIT_VERSION WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "^([0-9]+)\\..*" "\\1" VERSION_MAJOR "${GIT_VERSION}")
    string(REGEX REPLACE "^${VERSION_MAJOR}\\.([0-9]+).*" "\\1" VERSION_MINOR "${GIT_VERSION}")
    string(REGEX REPLACE "^${VERSION_MAJOR}\\.${VERSION_MINOR}\\.([0-9]+).*" "\\1" VERSION_BUGFIX "${GIT_VERSION}")
    string(REGEX REPLACE "^${VERSION_MAJOR}\\.${VERSION_MINOR}\\.${VERSION_BUGFIX}-([0-9]+)-.*" "\\1" VERSION_NEW_COMMITS "${GIT_VERSION}")
    string(REGEX REPLACE "^${VERSION_MAJOR}\\.${VERSION_MINOR}\\.${VERSION_BUGFIX}-${VERSION_NEW_COMMITS}-(.*)" "\\1" VERSION_SHA1 "${GIT_VERSION}")
    if("${VERSION_NEW_COMMITS}" STREQUAL "${GIT_VERSION}")
        set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUGFIX}")
        set(VERSION_NEW_COMMITS "")
        set(VERSION_SHA1 "")
    else()
        set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUGFIX}-${VERSION_NEW_COMMITS}-${VERSION_SHA1}")
    endif()
    if ("${VERSION_STRING}" STREQUAL "..")
        message(FATAL_ERROR "Version string missing. Did you git clone using the --depth parameter?")
    endif()
else()
  message(FATAL_ERROR "Git not found.")
endif()
endif() # NOT ANDROID
#-----------------------------------------------------------------------------
# Options for building

option(BUILD_VERBOSE       "Verbose build output." ON)

if(ANDROID)
  option(BUILD_DOCUMENTATION "Build the Doxygen documentation." OFF)
  option(BUILD_TESTS         "Build the unit tests." OFF)
else()
  option(BUILD_DOCUMENTATION "Build the Doxygen documentation." ON)
  option(BUILD_TESTS         "Build the unit tests." ON)
endif()

option(OT_STRICT           "Use pedantic compiler options." ON)
option(USE_CCACHE          "Use ccache." OFF)

option(BUILD_SHARED_LIBS   "Build shared libraries." ON)

option(RPM                 "Build a RPM" OFF)
option(DEB                 "Build a DEB" OFF)
set(PACKAGE_CONTACT        ""                             CACHE <TYPE>  "Package Maintainer")
set(PACKAGE_VENDOR         "Open Transactions Developers" CACHE <TYPE>  "Package Vendor")
option(AUTOCOMPLETION      "CL autocompletion to opentxs CL client" OFF)

option(KEYRING_WINDOWS     "Build with Windows Keyring" OFF)
option(KEYRING_MAC         "Build with Mac OSX Keyring" OFF)
option(KEYRING_IOS         "Build with iOS Keyring" OFF)
option(KEYRING_GNOME       "Build with Gnome Keyring" OFF)
option(KEYRING_KWALLET     "Build with KWallet Keyring" OFF)
option(KEYRING_FLATFILE    "Build with Flatfile Keyring" OFF)
option(NO_PASSWORD         "Use default password for all prompts" OFF)

if(ANDROID)
  option(OT_STORAGE_FS       "Use filesystem backend for storage" OFF)
  option(OT_STORAGE_SQLITE   "Use sqlite backend for storage" ON)
else()
  option(OT_STORAGE_FS       "Use filesystem backend for storage" ON)
  option(OT_STORAGE_SQLITE   "Use sqlite backend for storage" OFF)
endif()

option(OT_DHT    "Enable OpenDHT support" OFF)

option(OT_CRYPTO_SUPPORTED_ALGO_AES     "Enable AES encryption algorithm" ON)

option(OT_CRYPTO_SUPPORTED_SOURCE_BIP47     "Enable support for BIP-47 nyms" ON)

option(OT_CRYPTO_SUPPORTED_KEY_HD     "Enable HD key derivation support" ON)
option(OT_CRYPTO_SUPPORTED_KEY_ED25519     "Enable ed25519 key support" ON)
option(OT_CRYPTO_SUPPORTED_KEY_RSA     "Enable RSA key support" OFF)
option(OT_CRYPTO_SUPPORTED_KEY_SECP256K1 "Enable secp256k1 key support" ON)

option(OT_CRYPTO_USING_OPENSSL       "Use OpenSSL crypto implementation" ON)
option(OT_CRYPTO_USING_LIBSECP256K1  "Use libsecp256k1 crypto implementation" ON)
option(OT_CRYPTO_USING_TREZOR  "Use trezor crypto implementation" ON)
option(OT_CRYPTO_USING_LIBBITCOIN  "Enable libbitcoin support" OFF)

option(OT_CRYPTO_SHA2_PREFER_OPENSSL       "Use OpenSSL SHA2 implementation when available" OFF)

if(ANDROID)
  option(OT_SCRIPT_USING_CHAI  "Use chaiscript" OFF)
else()
  option(OT_SCRIPT_USING_CHAI  "Use chaiscript" ON)
endif()

option(OT_INTERNAL_LIBSECP   "Use bundled version of libsecp256k1" OFF)

# SWIG Bindings
if(ANDROID)
  option(JAVA                "Build with Java binding" ON)
else()
  option(JAVA                "Build with Java binding" OFF)
endif()
option(PERL    "Build with Perl binding" OFF)
option(PHP     "Build with PHP binding" OFF)
option(PYTHON3 "Build with Python3 binding" OFF)
option(PYTHON2 "Build with Python2 binding" OFF)
option(PYTHON  "Build with default Python binding" OFF)


if(BUILD_VERBOSE)
  set(CMAKE_VERBOSE_MAKEFILE true)
endif()

if(RPM OR DEB)
    if(NOT PACKAGE_CONTACT)
        message(FATAL_ERROR "Specify a PACKAGE_CONTACT to build packages.")
    endif()
endif()

#-----------------------------------------------------------------------------
# Print system information and build options

message(STATUS "General -------------------------------------")
message(STATUS "opentxs version:        ${VERSION_STRING}")
message(STATUS "CMake version:          ${CMAKE_VERSION}")
message(STATUS "System:                 ${CMAKE_SYSTEM}")
message(STATUS "Processor:              ${CMAKE_SYSTEM_PROCESSOR}")

message(STATUS "Verbose:                ${BUILD_VERBOSE}")
message(STATUS "Testing:                ${BUILD_TESTS}")
message(STATUS "Documentation:          ${BUILD_DOCUMENTATION}")
message(STATUS "Using ccache            ${USE_CCACHE}")
message(STATUS "Pedantic compilation:   ${OT_STRICT}")

message(STATUS "Packaging -----------------------------------")
message(STATUS "Build RPM:              ${RPM}")
message(STATUS "Build DEB:              ${DEB}")
message(STATUS "Package Contact:        ${PACKAGE_CONTACT}")
message(STATUS "Package Vendor:         ${PACKAGE_VENDOR}")

message(STATUS "CL autocompletion:      ${AUTOCOMPLETION}")

message(STATUS "Keyrings ------------------------------------")
message(STATUS "Keyring Windows:        ${KEYRING_WINDOWS}")
message(STATUS "Keyring MAC:            ${KEYRING_MAC}")
message(STATUS "Keyring iOS:            ${KEYRING_IOS}")
message(STATUS "Keyring Gnome:          ${KEYRING_GNOME}")
message(STATUS "Keyring KWallet:        ${KEYRING_KWALLET}")
message(STATUS "Keyring flatfile:       ${KEYRING_FLATFILE}")
message(STATUS "No password:            ${NO_PASSWORD}")

message(STATUS "Network plugins------------------------------")
message(STATUS "DHT:                    ${OT_DHT}")

message(STATUS "Storage backends-----------------------------")
message(STATUS "filesystem:             ${OT_STORAGE_FS}")
message(STATUS "sqlite                  ${OT_STORAGE_SQLITE}")

message(STATUS "Nym ID sources------------------------------")
message(STATUS "BIP-47:                 ${OT_CRYPTO_SUPPORTED_SOURCE_BIP47}")

message(STATUS "Encryption algorithms------------------------")
message(STATUS "AES:                    ${OT_CRYPTO_SUPPORTED_ALGO_AES}")

message(STATUS "Key algorithms-------------------------------")
message(STATUS "HD derivation:          ${OT_CRYPTO_SUPPORTED_KEY_HD}")
message(STATUS "ed25519:                ${OT_CRYPTO_SUPPORTED_KEY_ED25519}")
message(STATUS "RSA:                    ${OT_CRYPTO_SUPPORTED_KEY_RSA}")
message(STATUS "secp256k1               ${OT_CRYPTO_SUPPORTED_KEY_SECP256K1}")

message(STATUS "Crypto library providers---------------------")
message(STATUS "OpenSSL:                ${OT_CRYPTO_USING_OPENSSL}")
message(STATUS "libsecp256k1:           ${OT_CRYPTO_USING_LIBSECP256K1}")
message(STATUS "trezor-crypto:          ${OT_CRYPTO_USING_TREZOR}")
message(STATUS "libbitcoin:             ${OT_CRYPTO_USING_LIBBITCOIN}")

message(STATUS "Bindings ------------------------------------")
message(STATUS "Java binding:           ${JAVA}")
message(STATUS "Perl binding:           ${PERL}")
message(STATUS "PHP binding:            ${PHP}")
message(STATUS "Python3 binding:        ${PYTHON3}")
message(STATUS "Python2 binding:        ${PYTHON2}")
message(STATUS "Default Python binding: ${PYTHON}")

message(STATUS "Script engines-------------------------------")
message(STATUS "Chai:                   ${OT_SCRIPT_USING_CHAI}")

message(STATUS "Dependencies---------------------------------")
message(STATUS "Internal Libsecp256k1:  ${OT_INTERNAL_LIBSECP}")

#-----------------------------------------------------------------------------
# Collect all binaries into bin subdirectory

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH  ${CMAKE_CURRENT_BINARY_DIR}/lib)


#-----------------------------------------------------------------------------
# Check which compiler to use

INCLUDE(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(-std=c++17 HAVE_STD17)

if(HAVE_STD17)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
  set(GCC_WARN_FLAGS "-Wnoexcept -Wstrict-null-sentinel -Wlogical-op")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

  if(ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -frtti")
  endif()
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  set(CMAKE_COMPILER_IS_CLANGXX 1)

  if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -stdlib=libc++")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
  endif()
endif()

if(WIN32)
  add_definitions(-D_UNICODE)
else()
  if(OT_STRICT)

    set(PEDANTIC_CXX_FLAGS "-Wall -Werror -Wextra -pedantic -Wno-missing-braces -Wcast-align -Wctor-dtor-privacy -Wdisabled-optimization -Winit-self -Wmissing-include-dirs -Wswitch-default -Wold-style-cast -Woverloaded-virtual -Wmissing-declarations -Wno-deprecated-declarations -Wno-unused-parameter -Wno-old-style-cast -Wunused-macros -Wundef -Wformat=2 -Wno-format-nonliteral ${GCC_WARN_FLAGS}")
  else()
    set(PEDANTIC_CXX_FLAGS "")
  endif()
endif()

if(ANDROID)
  set(PEDANTIC_CXX_FLAGS "${PEDANTIC_CXX_FLAGS} -Wno-useless-cast -Wno-missing-field-initializers")
  add_definitions(-D__STDC_LIMIT_MACROS)
endif()

if(CMAKE_COMPILER_IS_CLANGXX)
  set(PEDANTIC_CXX_FLAGS "${PEDANTIC_CXX_FLAGS} -Wno-unused-parameter -Wno-old-style-cast")
endif()

message(STATUS "C++ Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C Flags:         ${CMAKE_C_FLAGS}")
message(STATUS "C++ Flags:       ${CMAKE_CXX_FLAGS}")
message(STATUS "C++ link flags:  ${CMAKE_CXX_LINK_FLAGS}")

#-----------------------------------------------------------------------------
# Get submodules

execute_process(COMMAND git "submodule" "update" "--init" "--recursive" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

#-----------------------------------------------------------------------------
# Third party libraries

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_definitions(-DPROTOBUF_INLINE_NOT_IN_HEADERS=0)
find_package(opentxs-proto REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(APPLE)
  message(STATUS "On Apple, you may have to use: brew install openssl ; brew link --force openssl")
  find_package(OpenSSL 1.0.1 REQUIRED)
else()
  find_package(OpenSSL 1.0.1 REQUIRED)
endif()
find_package(Protobuf REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Sodium REQUIRED)
find_package(czmq 4 REQUIRED)
if(OT_CRYPTO_USING_LIBSECP256K1 AND OT_INTERNAL_LIBSECP)
  find_package(GMP REQUIRED)
endif()
if(OT_DHT)
  find_package(OPENDHT 0.6 REQUIRED)
  find_package(GnuTLS REQUIRED)
endif()
if(OT_STORAGE_SQLITE)
  find_package(SQLite3 REQUIRED)
endif()
if(OT_STORAGE_FS)
  find_package(Boost REQUIRED filesystem)
  find_package(Boost REQUIRED iostreams)
  find_package(Boost REQUIRED system)
endif()
if (NOT OT_INTERNAL_LIBSECP)
  find_package(Libsecp256k1 REQUIRED)
endif()
if (OT_CRYPTO_USING_LIBBITCOIN)
  find_package(libbitcoin REQUIRED)
endif()

#-----------------------------------------------------------------------------
# System libraries used for linking.

if(UNIX AND NOT APPLE)
  list(APPEND OPENTXS_SYSTEM_LIBRARIES dl)
  if(ANDROID)
    list(APPEND OPENTXS_SYSTEM_LIBRARIES log)
  else()
    list(APPEND OPENTXS_SYSTEM_LIBRARIES rt)
  endif()
endif()


#-----------------------------------------------------------------------------
# Force out-of-source build

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
message(FATAL_ERROR "CMake generation is not allowed within the source directory!
Remove the CMakeCache.txt file and try again from another folder, e.g.:

   rm CMakeCache.txt
   mkdir build
   cd build
   cmake ..
")
endif()

#-----------------------------------------------------------------------------
# Use ccache

if (USE_CCACHE)
  find_program(CCACHE "ccache")
  if (CCACHE)
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "CCACHE_CPP2=yes ${CCACHE}")
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE})
  endif(CCACHE)
endif (USE_CCACHE)

#-----------------------------------------------------------------------------
# Default to Release build

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()


#-----------------------------------------------------------------------------
# Build Unit tests

if(BUILD_TESTS AND NOT ANDROID)
  set(GTEST_ROOT ${opentxs_SOURCE_DIR}/deps/gtest)
  set(GTEST_FOUND ON)
  set(GTEST_INCLUDE_DIRS ${GTEST_ROOT}/include)
  set(GTEST_LIBRARY gtest)
  set(GTEST_MAIN_LIBRARY gtest_main)
  set(GTEST_BOTH_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})

  enable_testing()
endif()


#-----------------------------------------------------------------------------
# Build Documentation

if(BUILD_DOCUMENTATION)
  set(DOC_GRAPHS "YES" CACHE STRING "Create dependency graphs (needs graphviz)")
  set(DOC_FULLGRAPHS "NO" CACHE STRING "Create call/callee graphs (large)")

  find_program(DOT_PATH dot)

  if (DOT_PATH STREQUAL "DOT_PATH-NOTFOUND")
    message("Doxygen: graphviz not found - graphs disabled")
    set(DOC_GRAPHS "NO")
  endif()

  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    configure_file("cmake/Doxyfile.in" "Doxyfile" @ONLY)
    configure_file("cmake/Doxygen.extra.css.in" "Doxygen.extra.css" @ONLY)
    add_custom_target(doc
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen.." VERBATIM)
  endif()
endif()


#-----------------------------------------------------------------------------
# Source Definitions

if(WIN32)
    if(NOT IDC_STATIC)
        add_definitions(-DIDC_STATIC=-1)
    endif()

    add_definitions(-DOT_COMPANYNAME="Open Transactions Community")
    add_definitions(-DOT_PRODUCTNAME="Open Transactions")
    add_definitions(-DOT_PRODUCTVERSION="${VERSION_MAJOR},${VERSION_MINOR},${VERSION_BUGFIX},0")
    add_definitions(-DOT_API_NAME="Open Transactions API")
    add_definitions(-DOT_LIBRARY_NAME="Open Transactions Library")
    add_definitions(-DOT_PROMPT_NAME="Open Transactions Prompt")
endif()

if(ANDROID)
  add_definitions(-DANDROID)
endif()

#Network

if(OT_DHT)
    set(DHT_EXPORT 1)
else()
    set(DHT_EXPORT 0)
endif()

#Storage backends

if(OT_STORAGE_FS)
  set(FS_EXPORT 1)
else()
  set(FS_EXPORT 0)
endif()

if(OT_STORAGE_SQLITE)
  set(SQLITE_EXPORT 1)
else()
  set(SQLITE_EXPORT 0)
endif()

if ((NOT OT_STORAGE_FS) AND (NOT OT_STORAGE_SQLITE))
  message(FATAL_ERROR "At least one storage backend must be defined.")
endif()

#Nym sources

if(OT_CRYPTO_SUPPORTED_SOURCE_BIP47)
  if (NOT OT_CRYPTO_SUPPORTED_KEY_HD)
    message(FATAL_ERROR "BIP-47 support requires HD key support")
  endif()
  if (NOT OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    message(FATAL_ERROR "BIP-47 support requires secp256k1 key support")
  endif()
  set(BIP47_EXPORT 1)
else()
  set(BIP47_EXPORT 0)
endif()

#Encryption algorithms

if (OT_CRYPTO_SUPPORTED_ALGO_AES)
  if (NOT OT_CRYPTO_USING_OPENSSL)
    message(FATAL_ERROR "AES support requires OpenSSL")
  endif()
  set(AES_EXPORT 1)
else()
  set(AES_EXPORT 0)
endif()

#Key types

if(OT_CRYPTO_SUPPORTED_KEY_HD)
  if (NOT OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    message(FATAL_ERROR "HD key support requires secp256k1 key support")
  endif()
  if (NOT OT_CRYPTO_USING_TREZOR)
    message(FATAL_ERROR "HD key support requires trezor-crypto")
  endif()
  set(HD_EXPORT 1)
else()
  set(HD_EXPORT 0)
endif()

if (NOT OT_CRYPTO_SUPPORTED_KEY_ED25519)
  set(ED25519_EXPORT 0)
  if (NOT OT_CRYPTO_SUPPORTED_KEY_RSA)
    if (NOT OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
        message(FATAL_ERROR "At least one type of key must be enabled")
    endif()
  endif()
else()
  set(ED25519_EXPORT 1)
endif()

if (OT_CRYPTO_SUPPORTED_KEY_RSA)
  if (NOT OT_CRYPTO_USING_OPENSSL)
    message(FATAL_ERROR "RSA support requires OpenSSL")
  endif()
  set(RSA_EXPORT 1)
else()
  set(RSA_EXPORT 0)
endif()

if (OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
  if (NOT OT_CRYPTO_USING_LIBSECP256K1)
    message(FATAL_ERROR "secp256k1 key support requires libsecp256k1")
  endif()
  if (NOT OT_CRYPTO_USING_TREZOR)
    message(FATAL_ERROR "secp256k1 key support requires trezor-crypto")
  endif()
  set(SECP256K1_EXPORT 1)
else()
  set(SECP256K1_EXPORT 0)
endif()

#Crypto libraries

if(OT_CRYPTO_USING_OPENSSL)
  set(OPENSSL_EXPORT 1)
  if(OT_CRYPTO_SHA2_PREFER_OPENSSL)
    set(SHA2_VIA_OPENSSL_EXPORT 1)
  else()
    set(SHA2_VIA_OPENSSL_EXPORT 0)
  endif()
else()
  set(OPENSSL_EXPORT 0)
  set(SHA2_VIA_OPENSSL_EXPORT 0)
endif()

if(OT_CRYPTO_USING_LIBSECP256K1)
  if (OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    set(LIBSECP256K1_EXPORT 1)
  else()
    set(LIBSECP256K1_EXPORT 0)
    message(WARNING "Libsecp256k1 is pointless without secp256k1 keys.")
  endif()
else()
  set(LIBSECP256K1_EXPORT 0)
endif()

if(OT_CRYPTO_USING_TREZOR)
    set(TREZORCRYPTO_EXPORT 1)
  if (OT_CRYPTO_SUPPORTED_KEY_HD OR OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    set(BIP32_EXPORT 1)
    set(BIP39_EXPORT 1)
  else()
    set(BIP32_EXPORT 0)
    set(BIP39_EXPORT 0)
  endif()
else()
  set(TREZORCRYPTO_EXPORT 0)
  set(BIP32_EXPORT 0)
  set(BIP39_EXPORT 0)
  message(FATAL_ERROR "trezor-crypto support is required for base58 encoding")
endif()

if(OT_CRYPTO_USING_LIBBITCOIN)
    set(LIBBITCOIN_EXPORT 1)
else()
  set(LIBBITCOIN_EXPORT 0)
endif()

if (TRUE)
    set(CASH_LUCRE_EXPORT 1)
else()
    set(CASH_LUCRE_EXPORT 0)
endif()

if (FALSE)
    set(CASH_MM_EXPORT 1)
else()
    set(CASH_MM_EXPORT 0)
endif()

#Keyrings
if(KEYRING_WINDOWS AND WIN32)
  add_definitions(-DOT_KEYRING_WINDOWS)
endif()

if(KEYRING_MAC AND APPLE)
  add_definitions(-DOT_KEYRING_MAC)
endif()

if(KEYRING_IOS)
  add_definitions(-DOT_KEYRING_IOS)
endif()

if(KEYRING_GNOME)
  add_definitions(-DOT_KEYRING_GNOME)
endif()

if(KEYRING_KWALLET)
  add_definitions(-DOT_KEYRING_KWALLET)
endif()

if(KEYRING_FLATFILE)
  add_definitions(-DOT_KEYRING_FLATFILE)
endif()

if(NO_PASSWORD)
  add_definitions(-DOT_NO_PASSWORD)
endif()

if(WIN32)
  add_definitions("-DEXPORT=__declspec(dllexport)")
else()
  add_definitions(-DEXPORT=)
endif()

#Script engines
if(OT_SCRIPT_USING_CHAI AND NOT ANDROID AND NOT KEYRING_IOS)
  set(SCRIPT_CHAI_EXPORT 1)
else()
  set(SCRIPT_CHAI_EXPORT 0)
endif()

#-----------------------------------------------------------------------------
# Set in-source Version
configure_file("Version.hpp" "${CMAKE_CURRENT_BINARY_DIR}/include/opentxs/Version.hpp" @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/opentxs/Version.hpp
        DESTINATION include/opentxs/
        COMPONENT dev)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

#-----------------------------------------------------------------------------
# Build source

#Fix RPATH
if(APPLE)
  set(CMAKE_MACOSX_RPATH 0)
  set(CMAKE_INSTALL_NAME_DIR @rpath)
endif()
if(NOT RPM)
  SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
  if(NOT WIN32)
    include(GNUInstallDirs)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
  endif()
endif()

if(UNIX)
  add_definitions(-DOT_PREFIX_PATH=\"${CMAKE_INSTALL_PREFIX}\")
endif()

function(set_lib_property lib)
    set_target_properties(${lib} PROPERTIES VERSION ${VERSION_STRING} SOVERSION ${VERSION_MAJOR})
endfunction(set_lib_property)

add_subdirectory(deps)

if (OT_CRYPTO_USING_LIBSECP256K1 AND OT_INTERNAL_LIBSECP)
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/deps/include)
    link_directories(${CMAKE_CURRENT_BINARY_DIR}/deps/lib)
endif()

if(NOT APPLE)
  add_definitions(-D_XOPEN_SOURCE=700)
endif()

add_subdirectory(src)
add_subdirectory(wrappers)

if (OT_CRYPTO_USING_LIBSECP256K1 AND OT_INTERNAL_LIBSECP)
    add_dependencies(opentxs-cron libsecp256k1)
    add_dependencies(opentxs-recurring libsecp256k1)
    add_dependencies(opentxs-script libsecp256k1)
    add_dependencies(opentxs-trade libsecp256k1)
    add_dependencies(opentxs-storage libsecp256k1)
    add_dependencies(opentxs-network libsecp256k1)
endif()

if (NOT ANDROID)
  add_subdirectory(tests)
endif()

if (NOT ANDROID)
#-----------------------------------------------------------------------------
# Produce a cmake-package

include(CMakePackageConfigHelpers)
set(CMAKE_CONFIG_DEST "share/cmake/Modules")
set(OPENTXS_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")

export(TARGETS opentxs FILE "${CMAKE_BINARY_DIR}/opentxsTargets.cmake")

configure_package_config_file(
       "cmake/opentxsConfig.cmake.in"
       "${CMAKE_BINARY_DIR}/opentxsConfig.cmake"
       INSTALL_DESTINATION ${CMAKE_CONFIG_DEST}
       PATH_VARS OPENTXS_INCLUDE_DIRS)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/opentxsConfigVersion.cmake"
  VERSION ${VERSION_STRING}
  COMPATIBILITY AnyNewerVersion)

install(EXPORT opentxsTargets
        DESTINATION ${CMAKE_CONFIG_DEST} FILE "opentxsTargets.cmake")

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/opentxsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/opentxsConfigVersion.cmake"
        DESTINATION ${CMAKE_CONFIG_DEST}
        COMPONENT dev)

#-----------------------------------------------------------------------------
# Produce a pkg-config file

if(NOT WIN32)
	configure_file("cmake/opentxs.pc.in" "opentxs.pc" @ONLY)
	include(GNUInstallDirs)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/opentxs.pc"
		DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
		COMPONENT main)
endif()

#-----------------------------------------------------------------------------
# Install

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include
        COMPONENT dev
        FILES_MATCHING PATTERN "*.hpp"
        PATTERN "stdafx.hpp" EXCLUDE)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/simple_ptr.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/containers_fixes.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/exceptions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/copy_functors.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/simple_ptr.tpp
    DESTINATION include/opentxs/core/containers
    COMPONENT dev)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/irrxml/
        DESTINATION include/irrxml
        COMPONENT dev
        FILES_MATCHING PATTERN "*.hpp")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ot/
        DESTINATION lib/opentxs
        COMPONENT dev
        FILES_MATCHING PATTERN "*.ot")

# opentxs CL client autocompletion
if(AUTOCOMPLETION)
  install(FILES
          ${CMAKE_CURRENT_SOURCE_DIR}/scripts/bash_completion.d/opentxs-bash-completion.sh
	  DESTINATION /etc/bash_completion.d/
          COMPONENT main)
endif()

#-----------------------------------------------------------------------------
# Uninstall
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake)

#-----------------------------------------------------------------------------
# Packaging
set(CPACK_GENERATOR "TGZ")
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

set(CPACK_PACKAGE_VERSION ${VERSION_STRING})
set(CPACK_PACKAGE_NAME "opentxs")
set(CPACK_PACKAGE_CONTACT ${PACKAGE_CONTACT})
set(CPACK_PACKAGE_VENDOR ${PACKAGE_VENDOR})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open Transactions library and command line client")
set(CPACK_PACKAGE_DESCRIPTION "
Open-Transactions democratizes financial and monetary actions.
You can use it for issuing currencies/stock, paying dividends, creating asset accounts, sending/receiving digital cash, writing/depositing cheques, cashier's cheques, creating basket currencies, trading on markets, scripting custom agreements, recurring payments, escrow, etc.
")

if(RPM)
  set(CPACK_GENERATOR "RPM")
  set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
  set(CPACK_RPM_COMPONENT_INSTALL ON)
  set(CPACK_RPM_CHANGELOG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog")
endif()

if(DEB)
  set(CPACK_GENERATOR "DEB")
  set(CPACK_DEB_COMPONENT_INSTALL ON)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "openssl, libprotobuf9")
  set(CPACK_DEBIAN_PACKAGE_PRIORITY "extra")
  set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
  set(CPACK_DEBIAN_PACKAGE_HOMEPAGE ${OPEN_TRANSACTIONS_WEBSITE})
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${PACKAGE_CONTACT})
endif()

include(CPack)
endif (NOT ANDROID)
