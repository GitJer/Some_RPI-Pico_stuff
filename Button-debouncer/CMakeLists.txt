add_executable(pio_button_debounce)

pico_generate_pio_header(pio_button_debounce ${CMAKE_CURRENT_LIST_DIR}/button_debounce.pio)

target_sources(pio_button_debounce PRIVATE button_debounce.cpp main.cpp)

target_link_libraries(pio_button_debounce PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(pio_button_debounce)

# add url via pico_set_program_url
example_auto_set_url(pio_button_debounce)


