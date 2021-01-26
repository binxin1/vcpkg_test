find_package(cpprestsdk CONFIG REQUIRED)

file(GLOB_RECURSE code_src
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

#add_library(main ${code_src})
add_executable(main ${code_src})

target_link_libraries(
    main
    PUBLIC
    cpprestsdk::cpprest
)

target_include_directories(
    main
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)