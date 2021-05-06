add_executable(onewire)

pico_generate_pio_header(onewire ${CMAKE_CURRENT_LIST_DIR}/onewire.pio)

target_sources(onewire PRIVATE onewire.cpp)

target_link_libraries(onewire PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(onewire)

# add url via pico_set_program_url
example_auto_set_url(onewire)


