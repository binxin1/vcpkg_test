find_package(CURL CONFIG REQUIRED)
find_package(cpprestsdk CONFIG REQUIRED)



#file(GLOB_RECURSE code_src
#    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
#    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
#)
#add_executable(curl_main ${code_src})

add_executable(curl_main curl_main.cpp)



target_link_libraries(
     curl_main
     PUBLIC
     CURL::libcurl
     PRIVATE
     cpprestsdk::cpprest
)

target_include_directories(
    curl_main
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

