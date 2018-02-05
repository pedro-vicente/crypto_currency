#CMake build, Pedro Vicente 
cmake_minimum_required(VERSION 2.8)
project (coin)
include(CheckCXXCompilerFlag)
set(BUILD_SHARED_LIBRARIES OFF)

#//////////////////////////
# colorized output
# red, fatal error, cannot build (missing netCDF, HDF5, CURL libraries)
# magenta, "negative" warning (missing optional libraries, GSL, ANTLR, UDUNITS)
# green, "positive" warning (opposite of magenta, optional libraries found)
# blue, general information to take notice (SZIP/ZLIB were detected as needed, special NCO functions are defined)
#//////////////////////////

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(color_reset   "${Esc}[m")
  set(color_magenta "${Esc}[35m")
  set(color_red     "${Esc}[31m")
  set(color_green   "${Esc}[32m")
  set(color_blue    "${Esc}[34m")
endif()


#///////////////////////////////////////////////////////
#MSVC
#///////////////////////////////////////////////////////

if (MSVC)
  add_definitions( "/D_CRT_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS" )
  if (STATIC_CRT)
    message("-- Building with static runtime library")
    set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1")
    set(CMAKE_CXX_FLAGS_MINSIZEREL     "/MT /O1 /Ob1 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "/MT /O2 /Ob2 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT /Zi /O2 /Ob1 /D NDEBUG")
  endif()  
endif(MSVC)

if (CMAKE_VERSION VERSION_LESS "3.1")
 if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
   CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
   if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
   elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
   else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
   endif()
 endif()
else ()
  set (CMAKE_CXX_STANDARD 11)
endif()

set(LIBS ${LIBS})
if (MSVC)
  set(LIBS ${LIBS} ws2_32.lib)
endif(MSVC)

include_directories(lib)
include_directories(gason/src)

set(sources ${sources})
set(sources ${sources} lib/socket.cc)
set(sources ${sources} lib/socket.hh)
set(sources ${sources} lib/http.hh)
set(sources ${sources} lib/http.cc)
set(sources ${sources} gason/src/gason.h)
set(sources ${sources} gason/src/gason.cpp)
set(sources ${sources} coin.hh)
set(sources ${sources} coin.cc)
add_executable(http_coin http_coin.cc ${sources})
target_link_libraries (http_coin ${LIBS})

#///////////////////////////////////////////////////////
#OpenSSL
#///////////////////////////////////////////////////////

if(OPENSSL_INCLUDE)
  if(EXISTS "${OPENSSL_INCLUDE}/openssl/ssl.h")
  else()
    message(FATAL_ERROR "${color_red}/openssl/ssl.h not found at ${OPENSSL_INCLUDE}${color_reset}")
  endif()
endif()

find_path(OPENSSL_INCLUDE openssl/ssl.h)
if(OPENSSL_INCLUDE)
  message("-- Found /openssl/ssl.h header file at: " ${OPENSSL_INCLUDE})
else()
  message("-- OpenSSL not found")
endif()

if (OPENSSL_LIBRARY)
  if(EXISTS "${OPENSSL_LIBRARY}")
  else()
    message(FATAL_ERROR "${color_red}Specified ${OPENSSL_LIBRARY} does not exist${color_reset}")
  endif()
endif()

if (OPENSSL_INCLUDE)

  find_library(OPENSSL_LIBRARY NAMES ssl libssl.lib)
  if(NOT OPENSSL_LIBRARY)
    message(FATAL_ERROR "${color_red}OpenSSL library not found${color_reset}")
  else()
    message("-- Found OpenSSL library at: " ${OPENSSL_LIBRARY})
  endif()

  if (CRYPTO_LIBRARY)
    if(EXISTS "${CRYPTO_LIBRARY}")
    else()
      message(FATAL_ERROR "${color_red}Specified ${CRYPTO_LIBRARY} does not exist${color_reset}")
    endif()
  endif()

  find_library(CRYPTO_LIBRARY NAMES crypto libcrypto.lib)
  if(NOT CRYPTO_LIBRARY)
    message(FATAL_ERROR "${color_red}CRYPTO library not found${color_reset}")
  else()
    message("-- Found CRYPTO library at: " ${CRYPTO_LIBRARY})
  endif()

  if(OPENSSL_INCLUDE AND OPENSSL_LIBRARY AND CRYPTO_LIBRARY)
    message("-- Building HTTPS coin")
    add_executable(ssl_client ssl_client.cc lib/ssl_socket.cc lib/ssl_socket.hh)
    add_executable(https_coin https_coin.cc lib/ssl_socket.cc lib/ssl_socket.hh ${sources})
    target_include_directories(ssl_client PRIVATE ${OPENSSL_INCLUDE})
    target_include_directories(https_coin PRIVATE ${OPENSSL_INCLUDE})
    if (MSVC)
      set(LIBS ${LIBS} crypt32.lib)
    else()
      set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pthread")
    endif(MSVC)
    target_link_libraries (ssl_client ${LIBS} ${OPENSSL_LIBRARY} ${CRYPTO_LIBRARY} ${CMAKE_DL_LIBS}) 
    target_link_libraries (https_coin ${LIBS} ${OPENSSL_LIBRARY} ${CRYPTO_LIBRARY} ${CMAKE_DL_LIBS}) 
  endif()

endif(OPENSSL_INCLUDE)

