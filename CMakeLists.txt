


project(assign02_project)

pico_sdk_init()

# Specify the name of the executable.
add_executable(assign02)

# Generate the PIO header file from the PIO source file.
pico_generate_pio_header(assign02 ${CMAKE_CURRENT_LIST_DIR}/assign02.pio)

# Specify the source files to be compiled.
target_sources(assign02 PRIVATE assign02.c assign02.S)

# Pull in commonly used features.
target_link_libraries(assign02 PRIVATE pico_stdlib hardware_pio)

# Include the PIO header file generated in the build directory.
target_include_directories(assign02 PRIVATE ${CMAKE_BINARY_DIR}/generated)

# Add the URL via pico_set_program_url.
apps_auto_set_url(assign02)

# Create map/bin/hex file etc.
pico_add_extra_outputs(assign02)