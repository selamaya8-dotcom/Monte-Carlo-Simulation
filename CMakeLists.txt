cmake_minimum_required(VERSION 3.16...3.21)
project(DroneMuonScan)

# Find Geant4 package
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all analysis)
else()
  find_package(Geant4 REQUIRED analysis)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include 
                    ${Geant4_INCLUDE_DIR})

# Locate sources
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

# Executable
add_executable(DroneMuonScan DroneMuonScan.cc ${sources} ${headers})

# Link Geant4
target_link_libraries(DroneMuonScan ${Geant4_LIBRARIES})

# Optional macro scripts
set(DroneMuonScan_SCRIPTS
  debug.mac
  vis.mac
)

foreach(_script ${DroneMuonScan_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
  )
endforeach()

# Install
install(TARGETS DroneMuonScan DESTINATION bin)
file(GLOB MACRO_FILES *.mac)
file(COPY ${MACRO_FILES} DESTINATION ${CMAKE_BINARY_DIR})

