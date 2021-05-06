add_executable(button_debounce)

pico_generate_pio_header(button_debounce ${CMAKE_CURRENT_LIST_DIR}/button_debounce.pio)

target_sources(button_debounce PRIVATE button_debounce.cpp)

target_link_libraries(button_debounce PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(button_debounce)

# add url via pico_set_program_url
example_auto_set_url(button_debounce)


