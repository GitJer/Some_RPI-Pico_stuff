add_executable(two_p_one_f)

pico_generate_pio_header(two_p_one_f ${CMAKE_CURRENT_LIST_DIR}/two_p_one_f.pio)

target_sources(two_p_one_f PRIVATE two_p_one_f.cpp)

target_link_libraries(two_p_one_f PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(two_p_one_f)

# add url via pico_set_program_url
example_auto_set_url(two_p_one_f)


