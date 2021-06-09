add_executable(subroutine)

pico_generate_pio_header(subroutine ${CMAKE_CURRENT_LIST_DIR}/subroutine.pio)

target_sources(subroutine PRIVATE subroutine.cpp)

target_link_libraries(subroutine PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(subroutine)
