add_executable(SBUS)

target_sources(SBUS PRIVATE SBUS.cpp)

target_link_libraries(SBUS PRIVATE
        pico_stdlib
        hardware_uart
        )

pico_add_extra_outputs(SBUS)


