cmake_minimum_required(VERSION 3.8)
project(loudness)
set(CMAKE_CXX_STANDARD 17)
set(JUCER_VST_LEGACY_SDK_FOLDER ${CMAKE_SOURCE_DIR}/vst2sdk)

set(JPACKAGE_NAME "loudmon")
set(PLUGIN_NAME "A1ex LoudMon")
set(PACKAGE_JUCER_FILE "${CMAKE_SOURCE_DIR}/loudmon/loudmon.jucer")
set(PLUGIN_CODE_INSTRUMENT 0x44713039)
set(PLUGIN_CODE_FX 0x4471303a)

if (WIN32)
  set(MSVC_COMPILE_FLAGS  "/MP")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_COMPILE_FLAGS}" )
endif()

if (BUILD_FRUT)
  set(JUCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/JUCE")
  add_subdirectory(3rdparty/FRUT)
else()
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/FRUT)
  execute_process(COMMAND cmake
      -DCMAKE_BUILD_TYPE=Release
      -DBUILD_FRUT=1
      -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/FRUT/install
	  ${CMAKE_CURRENT_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/FRUT
    COMMAND_ECHO STDOUT)

  execute_process(COMMAND cmake --build . --target install
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/FRUT
    COMMAND_ECHO STDOUT)

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

  if (UNIX)
    set(JUCER2CMAKE_EXECUTABLE Jucer2CMake)
  elseif(WIN32)
    set(JUCER2CMAKE_EXECUTABLE Jucer2CMake.exe)
  endif()
  execute_process(COMMAND ${CMAKE_CURRENT_BINARY_DIR}/FRUT/install/FRUT/bin/${JUCER2CMAKE_EXECUTABLE}
          reprojucer
          ${PACKAGE_JUCER_FILE}
          ${CMAKE_CURRENT_BINARY_DIR}/FRUT/install/FRUT/cmake/Reprojucer.cmake
          --relocatable
          --juce-modules ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/JUCE/modules
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/${JPACKAGE_NAME}
	RESULT_VARIABLE Jucer2CMakeStatus
    OUTPUT_VARIABLE Jucer2CMakeOutput
    ERROR_VARIABLE Jucer2CMakeError
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
	COMMAND_ECHO STDOUT)
  message(STATUS "Jucer2CMake: ")
  message(STATUS "  exit code: ${Jucer2CMakeStatus}")
  message(STATUS "  STDOUT: ${Jucer2CMakeOutput}")
  message(STATUS "  STDERR: ${Jucer2CMakeError}")
  message(STATUS "Jucer2CMake done")

  # Custom code starts
  # NOTE: You must use Ninja or Unix Makefiles generator
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif()
  set(${JPACKAGE_NAME}_jucer_FILE ${PACKAGE_JUCER_FILE})
  if (PLUGIN_TYPE STREQUAL "Instrument")
    add_compile_definitions(JucePlugin_IsSynth=1)
    add_compile_definitions(JucePlugin_Name="${PLUGIN_NAME}")
    add_compile_definitions(JucePlugin_PluginCode=${PLUGIN_CODE_INSTRUMENT})
    add_compile_definitions(JucePlugin_WantsMidiInput=1)
    add_compile_definitions(JucePlugin_VSTCategory=kPlugCategSynth)
    add_compile_definitions(JucePlugin_Vst3Category="Instrument")
  else()
    add_compile_definitions(JucePlugin_IsSynth=0)
    add_compile_definitions(JucePlugin_Name="${PLUGIN_NAME}")
    add_compile_definitions(JucePlugin_PluginCode=${PLUGIN_CODE_FX})
    add_compile_definitions(JucePlugin_WantsMidiInput=0)
    add_compile_definitions(JucePlugin_VSTCategory=kPlugCategEffect)
    add_compile_definitions(JucePlugin_Vst3Category="Fx")
  endif()

  add_subdirectory(${JPACKAGE_NAME})
endif()


