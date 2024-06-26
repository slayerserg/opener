#######################################
# Required CMake version              #
#######################################
cmake_minimum_required( VERSION 3.3 )

#######################################
# Project name                        #
#######################################
project( OpENer LANGUAGES C VERSION 2.3 )

#######################################
# Project version                     #
#######################################

if( NOT DEFINED OpENer_Device_Config_Vendor_Id )
  set( OpENer_Device_Config_Vendor_Id 1 CACHE STRING "Device Vendor ID" )
endif()
if( NOT DEFINED OpENer_Device_Config_Device_Type )
  set( OpENer_Device_Config_Device_Type 12 CACHE STRING "Device Type ID" )
endif()
if( NOT DEFINED OpENer_Device_Config_Product_Code )
  set( OpENer_Device_Config_Product_Code 65001 CACHE STRING "Device Product Code" )
endif()
if( NOT DEFINED OpENer_Device_Config_Device_Name )
  set( OpENer_Device_Config_Device_Name "OpENer PC" CACHE STRING "Device Name" )
endif()

set( OpENer_PLATFORM "POSIX" )

configure_file(  
	"${PROJECT_SOURCE_DIR}/src/ports/devicedata.h.in"
	"${PROJECT_BINARY_DIR}/src/ports/devicedata.h"
	)

set( OpENer_BUILDSUPPORT_DIR "${PROJECT_SOURCE_DIR}/buildsupport" )
#find_path( OpENer_BUILDSUPPORT_DIR OpENer.cmake ${PROJECT_SOURCE_DIR}/buildsupport )
 
INCLUDE( ${OpENer_BUILDSUPPORT_DIR}/OpENer.cmake )

option(OPENER_PRODUCED_DATA_HAS_RUN_IDLE_HEADER "Shall produced data from OpENer also include a run idle header?" FALSE)
option(OPENER_CONSUMED_DATA_HAS_RUN_IDLE_HEADER "Will consumed data from OpENer also include a run idle header?" TRUE)

if(OPENER_PRODUCED_DATA_HAS_RUN_IDLE_HEADER)
  add_definitions(-DOPENER_PRODUCED_DATA_HAS_RUN_IDLE_HEADER)
endif()

if(OPENER_CONSUMED_DATA_HAS_RUN_IDLE_HEADER)
  add_definitions(-DOPENER_CONSUMED_DATA_HAS_RUN_IDLE_HEADER)
endif()


#######################################
# Platform switches                   #
#######################################
set( OpENer_KNOWN_PLATFORMS "POSIX" "WIN32" "MINGW")

set( OpENer_PLATFORM CACHE STRINGS "Platform OpENer will be built for" )
set_property(CACHE OpENer_PLATFORM PROPERTY STRINGS ${OpENer_KNOWN_PLATFORMS} )

#######################################
# Platform-dependent functions check  #
#######################################
INCLUDE( ${OpENer_BUILDSUPPORT_DIR}/OpENer_function_checks.cmake )

#######################################
# OpENer 64-bit data types            #
#######################################
set( OpENer_64_BIT_DATA_TYPES_ENABLED CACHE BOOL "Enable 64-bit data type support" )
if( OpENer_64_BIT_DATA_TYPES_ENABLED )
  add_definitions( -DOPENER_SUPPORT_64BIT_DATATYPES )
endif( OpENer_64_BIT_DATA_TYPES_ENABLED )

#######################################
# OpENer tracer switches              #
#######################################
set( OpENer_TRACES OFF CACHE BOOL "Activate OpENer traces" )
if(OpENer_TRACES)
  createTraceLevelOptions()
endif(OpENer_TRACES)

#######################################
# Test switch                         #
#######################################
set( OpENer_TESTS OFF CACHE BOOL "Enable tests to be built" )
if( OpENer_TESTS )
  enable_testing()
  enable_language( CXX )
  set( CPPUTEST_HOME "" CACHE PATH "Path to CppUTest directory" )
  INCLUDE( ${OpENer_BUILDSUPPORT_DIR}/OpENer_Tests.cmake )
  INCLUDE( ${OpENer_BUILDSUPPORT_DIR}/CodeCoverage.cmake )
  set(COVERAGE_LCOV_EXCLUDES 'tests/*' 'usr/*' 'ports/POSIX/sample_application/*')
  SETUP_TARGET_FOR_COVERAGE_LCOV(NAME ${PROJECT_NAME}_coverage EXECUTABLE OpENer_Tests DEPENDENCIES OpENer_Tests)
  add_test_includes()
  add_subdirectory( tests )
endif( OpENer_TESTS )

#######################################
# OpENer C flags                      #
#######################################
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall" )

####################################################
# Internal cache holding the available CIP objects #
####################################################
set( OpENer_ADD_CIP_OBJECTS "" CACHE INTERNAL STRING )

#######################################
# Add custom CIP objects              #
#######################################
set( OpENer_CIP_OBJECTS_DIR ${PROJECT_SOURCE_DIR}/src/cip_objects )
include(${OpENer_BUILDSUPPORT_DIR}/OpENer_CIP_Object_generator.cmake)

#######################################
# Add subdirectories                  #
#######################################
add_subdirectory( src )
