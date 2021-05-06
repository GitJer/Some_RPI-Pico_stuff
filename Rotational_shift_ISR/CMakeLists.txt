add_executable(rotational_shift_ISR)

pico_generate_pio_header(rotational_shift_ISR ${CMAKE_CURRENT_LIST_DIR}/rotational_shift_ISR.pio)

target_sources(rotational_shift_ISR PRIVATE rotational_shift_ISR.cpp)

target_link_libraries(rotational_shift_ISR PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(rotational_shift_ISR)

# add url via pico_set_program_url
example_auto_set_url(rotational_shift_ISR)


