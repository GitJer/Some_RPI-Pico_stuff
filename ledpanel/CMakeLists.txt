add_executable(ledpanel)

pico_generate_pio_header(ledpanel ${CMAKE_CURRENT_LIST_DIR}/ledpanel.pio)

target_sources(ledpanel PRIVATE ledpanel.c ledpanel_worker.c)

target_link_libraries(ledpanel PRIVATE
        pico_stdlib
        hardware_pio
        hardware_dma
        hardware_irq
        hardware_interp
        pico_multicore
        )
