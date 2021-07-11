add_executable(SBUS)

pico_generate_pio_header(SBUS ${CMAKE_CURRENT_LIST_DIR}/SBUS.pio)

target_sources(SBUS PRIVATE SBUS.cpp)

target_link_libraries(SBUS PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(SBUS)


