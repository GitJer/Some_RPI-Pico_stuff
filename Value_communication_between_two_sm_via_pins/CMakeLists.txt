add_executable(value_communication_between_two_sm_via_pins)

pico_generate_pio_header(value_communication_between_two_sm_via_pins ${CMAKE_CURRENT_LIST_DIR}/value_communication_between_two_sm_via_pins.pio)

target_sources(value_communication_between_two_sm_via_pins PRIVATE value_communication_between_two_sm_via_pins.cpp)

target_link_libraries(value_communication_between_two_sm_via_pins PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(value_communication_between_two_sm_via_pins)

# add url via pico_set_program_url
example_auto_set_url(value_communication_between_two_sm_via_pins)


