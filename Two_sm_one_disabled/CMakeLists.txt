add_executable(two_sm_one_disabled)

pico_generate_pio_header(two_sm_one_disabled ${CMAKE_CURRENT_LIST_DIR}/two_sm_one_disabled.pio)

target_sources(two_sm_one_disabled PRIVATE two_sm_one_disabled.cpp)

target_link_libraries(two_sm_one_disabled PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(two_sm_one_disabled)

# add url via pico_set_program_url
example_auto_set_url(two_sm_one_disabled)


