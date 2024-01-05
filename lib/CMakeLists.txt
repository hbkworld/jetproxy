cmake_minimum_required(VERSION 3.14)

set( INTERFACE_INCLUDE_DIR ../include/jetproxy)

set (JET_PROXY_INTERFACE_HEADERS
    ${INTERFACE_INCLUDE_DIR}/AnalogVariableHandler.hpp
    ${INTERFACE_INCLUDE_DIR}/BaseIntrospectionHandler.hpp
    ${INTERFACE_INCLUDE_DIR}/DataType.hpp
    ${INTERFACE_INCLUDE_DIR}/DelayedSaver.hpp
    ${INTERFACE_INCLUDE_DIR}/ErrorCode.hpp
    ${INTERFACE_INCLUDE_DIR}/Error.hpp
    ${INTERFACE_INCLUDE_DIR}/Event.hpp
    ${INTERFACE_INCLUDE_DIR}/Introspection.hpp
    ${INTERFACE_INCLUDE_DIR}/IntrospectionVariableHandler.hpp
    ${INTERFACE_INCLUDE_DIR}/JetProxy.hpp
    ${INTERFACE_INCLUDE_DIR}/JsonSchema.hpp
    ${INTERFACE_INCLUDE_DIR}/Method.hpp   
    ${INTERFACE_INCLUDE_DIR}/NumericVariableHandler.hpp
    ${INTERFACE_INCLUDE_DIR}/EnumValueHandler.hpp
    ${INTERFACE_INCLUDE_DIR}/ProxyJetStates.hpp
    ${INTERFACE_INCLUDE_DIR}/SelectionValueHandler.hpp
    ${INTERFACE_INCLUDE_DIR}/StringEnum.hpp
    ${INTERFACE_INCLUDE_DIR}/TypeFactory.hpp
)

set (JET_PROXY_SOURCES
    ${JET_PROXY_INTERFACE_HEADERS}
    DelayedSaver.cpp
    Error.cpp
    ErrorCode.cpp
    Event.cpp
    Introspection.cpp
    JetProxy.cpp
    JsonSchema.cpp
    Method.cpp
    EnumValueHandler.cpp
    ProxyJetStates.cpp
    SelectionValueHandler.cpp
    StringEnum.cpp
    TypeFactory.cpp
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(${PROJECT_NAME}
    ${JET_PROXY_SOURCES}
)

add_library(hbk::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC jsoncpp_lib jet::jetpeer jet::jetpeerasync)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR}
  VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}"
)


target_include_directories(${PROJECT_NAME} PUBLIC
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../include>
)

# Turn on all warnings as errors
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wall -Werror -fno-common -Wshadow -Winit-self -Wstrict-overflow=5 -Wunused-result -Wcast-qual -Wcast-align -Wpointer-arith -Wformat=2 -Wwrite-strings -Wmissing-declarations -pedantic
          ${ASAN_COMPILE_FLAGS}>
    $<$<CXX_COMPILER_ID:MSVC>:
          /W4>
)


set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD_REQUIRED ON
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
)

configure_file("pkg-config/${PROJECT_NAME}.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/pkg-config/${PROJECT_NAME}.pc" @ONLY)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/pkg-config/${PROJECT_NAME}.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  DIRECTORY "../include/${PROJECT_NAME}"
  DESTINATION include
)

install(
  DIRECTORY "../include/objectmodel"
  DESTINATION include
)

write_basic_package_version_file(
  "${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${${PROJECT_NAME}_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# Create and install stuff for find_package
install(
  EXPORT ${PROJECT_NAME}
  NAMESPACE hbk::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  FILE ${PROJECT_NAME}Config.cmake
)

