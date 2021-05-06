add_executable(two_sm_simple)

pico_generate_pio_header(two_sm_simple ${CMAKE_CURRENT_LIST_DIR}/two_sm_simple.pio)

target_sources(two_sm_simple PRIVATE two_sm_simple.cpp)

target_link_libraries(two_sm_simple PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(two_sm_simple)

# add url via pico_set_program_url
example_auto_set_url(two_sm_simple)


