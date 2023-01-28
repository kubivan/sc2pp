
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
    ../include/sc2pp/Map.h 
    Map.cpp
    ../include/sc2pp/Actions.h
    ../include/sc2pp/Agent.h
    ../include/sc2pp/Common.h
    ../include/sc2pp/Observation.h
    ../include/sc2pp/Query.h
    ../include/sc2pp/SC2Client.h
    ../include/sc2pp/SC2Context.h
    ../include/sc2pp/SC2IDs.h
    ../include/sc2pp/Planner.h
    Actions.cpp
    Agent.cpp
    Converters.cpp
    Converters.h
    Observation.cpp
    SC2Client.cpp
    SC2Context.cpp
    SC2Session.cpp
    SC2Session.h
    TestAgent.cpp
    TestAgent.h
    UnitPool.h
    main.cpp
    Planner.cpp
    Query.cpp

    utils/GridView.cpp
    utils/UnitTraits.cpp
    utils/UnitQuery.cpp

    ../include/sc2pp/utils/Footprints.h
    ../include/sc2pp/utils/Grid.h
    ../include/sc2pp/utils/GridUtils.h
    ../include/sc2pp/utils/GridView.h
    ../include/sc2pp/utils/GridView.hpp
    ../include/sc2pp/utils/UnitQuery.h
    ../include/sc2pp/utils/UnitTraits.h
)

add_executable(sc2pp ${SC2PP_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(sc2pp PRIVATE "." "../include/sc2pp" "../include/sc2pp/utils" ${Boost_INCLUDE_DIRS} PUBLIC "../include" "../external/magic_enum/include")
target_compile_features(sc2pp PRIVATE cxx_std_20)
#include_directories( ${Boost_INCLUDE_DIRS} )
target_link_libraries(sc2pp PUBLIC sc2proto PRIVATE Boost::filesystem)
set_property(TARGET sc2pp PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
