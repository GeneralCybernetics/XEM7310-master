cmake_minimum_required(VERSION 3.10)

# Name of the project
project(rhx)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Add the okFrontPanel library directory
link_directories(${CMAKE_SOURCE_DIR}/lib)

# Add the library
add_library(rhxcontroller STATIC src/rhxcontroller.cpp)
add_library(rhxdatablock STATIC src/rhxdatablock.cpp)
add_library(rhxregisters STATIC src/rhxregisters.cpp)

# Specify the output directory for the built library
set_target_properties(rhxcontroller PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
set_target_properties(rhxdatablock PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
set_target_properties(rhxregisters PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Link the okFrontPanel library to your targets
target_link_libraries(rhxcontroller okFrontPanel)
target_link_libraries(rhxdatablock okFrontPanel)
target_link_libraries(rhxregisters okFrontPanel)

# Add include directories for the C++ standard library
include_directories(${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

# Install target to specify where to install the library and header files
install(TARGETS rhxcontroller
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
install(TARGETS rhxdatablock
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
install(TARGETS rhxregisters
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)     

install(DIRECTORY include/ DESTINATION include)
