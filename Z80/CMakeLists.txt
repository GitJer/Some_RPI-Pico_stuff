add_executable(Z80)

pico_generate_pio_header(Z80 ${CMAKE_CURRENT_LIST_DIR}/Z80.pio)

target_sources(Z80 PRIVATE Z80.c)

target_link_libraries(Z80 PRIVATE
        pico_stdlib
        hardware_pio
        hardware_dma
        hardware_irq
        )

# pico_add_extra_outputs(Z80)

# add url via pico_set_program_url
# example_auto_set_url(Z80)


