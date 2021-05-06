add_executable(PwmIn)

pico_generate_pio_header(PwmIn ${CMAKE_CURRENT_LIST_DIR}/PwmIn.pio)

target_sources(PwmIn PRIVATE PwmIn.cpp)

target_link_libraries(PwmIn PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(PwmIn)

# add url via pico_set_program_url
example_auto_set_url(PwmIn)


