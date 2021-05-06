add_executable(blow_led)

target_sources(blow_led PRIVATE blow_led.cpp)

target_link_libraries(blow_led PRIVATE
        pico_stdlib
        hardware_adc
        hardware_dma
        )
        

