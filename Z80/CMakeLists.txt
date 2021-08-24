add_executable(Z80)

pico_generate_pio_header(Z80 ${CMAKE_CURRENT_LIST_DIR}/Z80.pio)

target_sources(Z80 PRIVATE Z80.c)

target_link_libraries(Z80 PRIVATE
        pico_stdlib
        hardware_pio
        hardware_irq
        hardware_vreg
        )


