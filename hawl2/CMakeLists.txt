cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(HawlSIM)
#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default


find_package(Geant4 REQUIRED ui_all vis_all)

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project


include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
include_directories(${PROJECT_SOURCE_DIR}/include 
                    ${Geant4_INCLUDE_DIR})

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
## Hawl is the 실행파일 name
add_executable(Hawl Hawl.cc ${sources} ${headers})
target_link_libraries(Hawl ${Geant4_LIBRARIES})


set(Hawl_SCRIPTS
  
  init_vis.mac
  gun.mac
  vis.mac
  
  )

foreach(_script ${Hawl_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

add_custom_target(HawlSIM DEPENDS Hawl)
