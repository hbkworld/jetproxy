cmake_minimum_required(VERSION 3.14)

project(example_jet_proxy LANGUAGES CXX)

find_package(Threads)

add_executable(jet_proxy_example main.cpp JetObjectProxy.cpp JetObjectProxyWithType.cpp JetStaticObjectProxyWithType.cpp JetObjectProxyWithSubObjectType.cpp)
add_executable(EnumValuesExample EnumValuesExample.cpp EnumValuesProxy.cpp)

target_link_libraries(jet_proxy_example PUBLIC hbk::jetproxy)

get_property(targets DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
foreach(tgt ${targets})
  target_include_directories(${tgt} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/msservice
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../include>
  )
  get_target_property(target_type ${tgt} TYPE)
  if (target_type STREQUAL "EXECUTABLE")

    target_compile_features(${tgt}
          PRIVATE cxx_std_17 cxx_auto_type)
    # Turn on all warnings as errors
    target_compile_options(${tgt} PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wall -Werror
          ${ASAN_COMPILE_FLAGS}>
     $<$<CXX_COMPILER_ID:MSVC>:
          /W4>)

    target_link_libraries(${tgt} PUBLIC PRIVATE ${CMAKE_THREAD_LIBS_INIT} jet::jetpeerasync jetproxy hbk::hbk jsoncpp_lib )
    install(TARGETS ${tgt} RUNTIME DESTINATION bin)
  endif()
  set_target_properties(${tgt} PROPERTIES
    LINK_FLAGS "${EXTRA_LINK_FLAGS}"
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
  )
endforeach()

