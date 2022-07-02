
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.77.0 REQUIRED COMPONENTS filesystem)
message(STATUS "Boost version: ${Boost_VERSION}")

set(SC2PP_SOURCES 
    main.cpp
    Sc2Session.h
    Sc2Session.cpp
)
add_executable(sc2pp ${SC2PP_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(sc2pp PRIVATE ${Boost_INCLUDE_DIRS})
target_compile_features(sc2pp PRIVATE cxx_std_20)
#include_directories( ${Boost_INCLUDE_DIRS} )
target_link_libraries(sc2pp PUBLIC sc2proto rxcpp PRIVATE Boost::filesystem)
set_property(TARGET sc2pp PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
