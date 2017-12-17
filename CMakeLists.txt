
cmake_minimum_required(VERSION 3.5)

project(chatrobot)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/CMake")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(Portaudio REQUIRED)
find_package(Threads REQUIRED)

include_directories(
    ${PORTAUDIO_INCLUDE_DIRS}
    "${CMAKE_SOURCE_DIR}/xf/include")

link_directories(
    "${CMAKE_SOURCE_DIR}/xf/linux_x64/libs")

add_executable(chatrobot
    main.cpp
    init_audio.cpp
    record.cpp
    play.cpp
    dialog.cpp
    recognize.cpp
    generate.cpp
    init_threads.cpp)

target_link_libraries(chatrobot
    ${PORTAUDIO_LIBRARIES}
    Threads::Threads
    "msc")

if(MSVC)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -flto")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
endif()
