
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.77.0 REQUIRED COMPONENTS filesystem)
message(STATUS "Boost version: ${Boost_VERSION}")

if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.12.25835)
    set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
    set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")
endif()

set(SC2PP_SOURCES 
    Actions.cpp
    Actions.h
    SC2Client.cpp
    SC2Client.h
    SC2IDs.h
    SC2Session.cpp
    SC2Session.h
    main.cpp
)

add_executable(sc2pp ${SC2PP_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(sc2pp PRIVATE ${Boost_INCLUDE_DIRS})
target_compile_features(sc2pp PRIVATE cxx_std_20)
#include_directories( ${Boost_INCLUDE_DIRS} )
target_link_libraries(sc2pp PUBLIC sc2proto rxcpp PRIVATE Boost::filesystem)
set_property(TARGET sc2pp PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
