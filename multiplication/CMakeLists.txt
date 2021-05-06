add_executable(multiplier)

pico_generate_pio_header(multiplier ${CMAKE_CURRENT_LIST_DIR}/multiplier.pio)

target_sources(multiplier PRIVATE multiplier.cpp)

target_link_libraries(multiplier PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(multiplier)

# add url via pico_set_program_url
example_auto_set_url(multiplier)


