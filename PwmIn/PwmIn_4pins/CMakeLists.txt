add_executable(PWM4)

pico_generate_pio_header(PWM4 ${CMAKE_CURRENT_LIST_DIR}/PwmIn.pio)

target_sources(PWM4 PRIVATE PWM4.cpp PwmIn.cpp)

target_link_libraries(PWM4 PRIVATE
        pico_stdlib
        hardware_pio
        hardware_pwm
        hardware_gpio
        )

pico_add_extra_outputs(PWM4)




