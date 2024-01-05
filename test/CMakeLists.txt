cmake_minimum_required(VERSION 3.14)

set(gtest_REQUIREDVERSION "1.13.0")

  find_package(GTest ${gtest_REQUIREDVERSION} )

  if(GTest_FOUND)
    message(STATUS "Found GTest: ${GTest_VERSION} ${GTest_CONFIG}")
  else()
    include(FetchContent)
    set(gtest_WITH_POST_BUILD_UNITTEST OFF)
    set(gtest_WITH_TESTS OFF)
    FetchContent_Declare(
      GTest
      URL https://github.com/google/googletest/archive/refs/tags/v${gtest_REQUIREDVERSION}.zip
    )
    FetchContent_GetProperties(GTest)
    if(NOT gtest_POPULATED)
      message(STATUS "Fetching GTest ${gtest_REQUIREDVERSION}...")
      FetchContent_Populate(GTest)
      add_subdirectory(${gtest_SOURCE_DIR} ${gtest_BINARY_DIR})
  endif()
    link_directories("${gtest_BINARY_DIR}")
    include_directories("${gtest_SOURCE_DIR}")
    add_library(GTest::GTest ALIAS gtest)
    add_library(GTest::Main ALIAS gtest_main)
  endif()



if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CGOV_COMPILE_FLAGS -fno-omit-frame-pointer -fno-optimize-sibling-calls -ftest-coverage -fprofile-arcs)
  set(GCOV_LINK_FLAGS -fprofile-arcs -ftest-coverage)
  set(GCOV_LIB gcov)
  set(ASAN_COMPILE_FLAGS -fsanitize=address -fno-omit-frame-pointer)
  set(ASAN_LIB asan)
endif()


# Create OBJECT_DIR variable
set(OBJECT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/jetservice.dir)
message("-- Object files will be output to: ${OBJECT_DIR}")

include_directories(..)
include_directories(../include/fbinterface)
include_directories(../include)

add_custom_target(coverage
  COMMAND echo "=================== Run ===================="
  COMMAND ${CMAKE_MAKE_PROGRAM} test ARGS="--output-on-failure"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

set_target_properties(coverage
  PROPERTIES
  ADDITIONAL_CLEAN_FILES "${CMAKE_BINARY_DIR}/*.html"
)


set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES coverage)


add_library(jetproxytestlib OBJECT
  ${JETPROXY_TEST_LIB}
  ../example/EnumValuesProxy.cpp
  ../example/SelectionValuesProxy.cpp
  ../example/JetObjectProxyWithSubObjectType.cpp
  ../example/SelectionValuesProxy.cpp
  ../lib/DelayedSaver.cpp
  ../lib/ErrorCode.cpp
  ../lib/Introspection.cpp
  ../lib/JetProxy.cpp
  ../lib/JsonSchema.cpp
  ../lib/Method.cpp
  ../lib/EnumValueHandler.cpp
  ../lib/ProxyJetStates.cpp
  ../lib/SelectionValueHandler.cpp
  ../lib/StringEnum.cpp
  ../lib/Event.cpp
  ../lib/TypeFactory.cpp
)

target_compile_options(jetproxytestlib PRIVATE
 $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
  -Wall -Werror ${CGOV_COMPILE_FLAGS} ${ASAN_COMPILE_FLAGS}>
)
set_target_properties(jetproxytestlib PROPERTIES
  CXX_STANDARD_REQUIRED ON
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
)

target_link_options(jetproxytestlib PRIVATE ${GCOV_LINK_FLAGS} )
target_link_libraries(
  jetproxytestlib
  PUBLIC
  hbk::jetproxy
  ${ASAN_LIB}
  ${GCOV_LIB}
)




enable_testing()


# The tests ==============
add_executable(ErrorCode.test ErrorCodeTest.cpp)
add_executable(Method.test MethodTest.cpp)
add_executable(StringEnum.test StringEnumTest.cpp)
add_executable(JetProxy.test JetProxyTest.cpp)
add_executable(JsonSchema.test JsonSchemaTest.cpp)
add_executable(Event.test EventTest.cpp)
add_executable(ObjectTypeTest.test ObjectTypeTest.cpp)
add_executable(DataTypeTest.test DataTypeTest.cpp)
add_executable(DelayedSaver.test DelayedSaverTest.cpp)
add_executable(EnumValues.test EnumValuesTest.cpp)
add_executable(Introspection.test IntrospectionTest.cpp)
add_executable(SelectionValues.test SelectionValuesTest.cpp)


# =========================


get_property(targets DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
foreach(tgt ${targets})
  get_target_property(target_type ${tgt} TYPE)
  if (target_type STREQUAL "EXECUTABLE")
    target_compile_options(${tgt} PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wall -Werror ${CGOV_COMPILE_FLAGS} ${ASAN_COMPILE_FLAGS}>
    )
# requires cmake 3.13
    target_link_options(${tgt} PRIVATE ${GCOV_LINK_FLAGS} )
    target_link_libraries(
      ${tgt}
      ${ASAN_LIB}
      ${GCOV_LIB}
      jetproxytestlib
      jet::jetpeerasync
      jet::jetpeer
      jsoncpp_lib
      hbk::hbk
      GTest::GTest GTest::Main
    )

    set_target_properties(${tgt} PROPERTIES
      CXX_STANDARD_REQUIRED ON
      CXX_STANDARD 17
      CXX_EXTENSIONS OFF
    )
    add_test(NAME Hbk.JetProxy.${tgt} COMMAND ${tgt})
    add_dependencies(coverage ${tgt})
  endif()
endforeach()


# note: cmake replaces ' ' in string with '\ ' creating a list solves this problem
set(COMMON_BRANCH_OPTIONS "--branches" "--exclude-unreachable-branches" "--exclude-throw-branches")




add_custom_command(TARGET coverage
  COMMAND gcovr -r ${CMAKE_SOURCE_DIR} -e ${CMAKE_SOURCE_DIR}/test -e ${CMAKE_SOURCE_DIR}/example -e ${CMAKE_BINARY_DIR}/_deps --object-dir ${CMAKE_BINARY_DIR} ${COMMON_BRANCH_OPTIONS} --html-details -o ${CMAKE_BINARY_DIR}/coverage.html
  COMMAND gcovr -r ${CMAKE_SOURCE_DIR} -e ${CMAKE_SOURCE_DIR}/test -e ${CMAKE_SOURCE_DIR}/example -e ${CMAKE_BINARY_DIR}/_deps --object-dir ${CMAKE_BINARY_DIR} ${COMMON_BRANCH_OPTIONS} --json -o ${CMAKE_BINARY_DIR}/coverage.json
  COMMAND gcovr -r ${CMAKE_SOURCE_DIR} -e ${CMAKE_SOURCE_DIR}/test -e ${CMAKE_SOURCE_DIR}/example -e ${CMAKE_BINARY_DIR}/_deps --object-dir ${CMAKE_BINARY_DIR} ${COMMON_BRANCH_OPTIONS}
  )

