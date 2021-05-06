add_executable(HCSR04)

pico_generate_pio_header(HCSR04 ${CMAKE_CURRENT_LIST_DIR}/HCSR04.pio)

target_sources(HCSR04 PRIVATE HCSR04.cpp)

target_link_libraries(HCSR04 PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(HCSR04)

# add url via pico_set_program_url
example_auto_set_url(HCSR04)


