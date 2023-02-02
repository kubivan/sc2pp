
set(SC2PP_TEST_SOURCES 
    Planner.h
    Planner.cpp
    TestAgent.cpp
    TestAgent.h
    main.cpp
)

add_executable(test-bot ${SC2PP_TEST_SOURCES})
target_link_libraries(test-bot sc2pp)