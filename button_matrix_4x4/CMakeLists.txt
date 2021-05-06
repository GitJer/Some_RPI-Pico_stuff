add_executable(4x4_button_matrix)

pico_generate_pio_header(4x4_button_matrix ${CMAKE_CURRENT_LIST_DIR}/4x4_button_matrix.pio)

target_sources(4x4_button_matrix PRIVATE 4x4_button_matrix.cpp)

target_link_libraries(4x4_button_matrix PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(4x4_button_matrix)

# add url via pico_set_program_url
example_auto_set_url(4x4_button_matrix)


