add_library(usermod_debounce INTERFACE)

target_sources(usermod_debounce INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/button_debounce.cpp
    ${CMAKE_CURRENT_LIST_DIR}/debounce.cpp
    ${CMAKE_CURRENT_LIST_DIR}/debouncemodule.c
)

target_include_directories(usermod_debounce INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(usermod INTERFACE usermod_debounce) 
