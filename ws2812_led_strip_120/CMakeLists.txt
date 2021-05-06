add_executable(ws2812_led_strip_120)

pico_generate_pio_header(ws2812_led_strip_120 ${CMAKE_CURRENT_LIST_DIR}/ws2812_led_strip_120.pio)

target_sources(ws2812_led_strip_120 PRIVATE ws2812_led_strip_120.c)

target_link_libraries(ws2812_led_strip_120 PRIVATE
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(ws2812_led_strip_120)

# add url via pico_set_program_url
example_auto_set_url(ws2812_led_strip_120)


