cmake_minimum_required(VERSION 3.14)

project(jetproxy VERSION 2.0.4 LANGUAGES CXX)

option(JETPROXY_POST_BUILD_UNITTEST "Automatically run unit-tests as a post build step" OFF)
option(JETPROXY_EXAMPLES            "Build examples of usages"                          OFF)

include(FetchContent)

set(jsoncpp_REQUIREDVERSION "1.9.3")
find_package(jsoncpp ${jsoncpp_REQUIREDVERSION} QUIET)
if(jsoncpp_FOUND)
  message(STATUS "Found jsoncpp: ${jsoncpp_VERSION} ${jsoncpp_CONFIG}")
else()
  set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF)
  set(JSONCPP_WITH_TESTS OFF)
  set(JSONCPP_WITH_STRICT_ISO OFF)
  message(STATUS "Fetching jsoncpp ${jsoncpp_REQUIREDVERSION}...")
  FetchContent_Declare(
    jsoncpp
    URL https://github.com/open-source-parsers/jsoncpp/archive/${jsoncpp_REQUIREDVERSION}.zip
  )
  FetchContent_MakeAvailable(jsoncpp)
endif()

set(hbk_REQUIREDVERSION "2.0.1")
find_package(hbk ${hbk_REQUIREDVERSION} QUIET)
if(hbk_FOUND)
  message(STATUS "Found hbk: ${hbk_VERSION} ${hbk_CONFIG}")
else(hbk_FOUND)
  set(hbk_POST_BUILD_UNITTEST OFF)
  message(STATUS "Fetching hbk ${hbk_REQUIREDVERSION}...")
  FetchContent_Declare(
    hbk
    URL https://github.com/hbkworld/libhbk/archive/refs/tags/v${hbk_REQUIREDVERSION}.tar.gz
  )
  FetchContent_MakeAvailable(hbk)
endif(hbk_FOUND)

set(jetpeer_REQUIREDVERSION "3.0.3")
find_package(jetpeerasync ${jetpeer_REQUIREDVERSION} QUIET)
find_package(jetpeer ${jetpeer_REQUIREDVERSION} QUIET)
if(jetpeer_FOUND AND jetpeerasync_FOUND)
    message(STATUS "Found jetpeer: ${jetpeer_VERSION} ${jetpeer_CONFIG}")
    message(STATUS "Found jetpeerasync: ${jetpeerasync_VERSION} ${jetpeerasync_CONFIG}")
else()
  set(JETPEER_POST_BUILD_UNITTEST OFF)
  message(STATUS "Fetching jetpeer ${jetpeer_REQUIREDVERSION}...")
  FetchContent_Declare(
    jetpeer
    URL https://github.com/hbkworld/jetpeer-cpp/archive/refs/tags/v${jetpeer_REQUIREDVERSION}.tar.gz
  )
  FetchContent_MakeAvailable(jetpeer)
endif()

add_subdirectory("lib")

if(JETPROXY_EXAMPLES)
  add_subdirectory("example")
endif()
if(JETPROXY_POST_BUILD_UNITTEST)
  enable_testing()
  add_subdirectory("test")
endif()
