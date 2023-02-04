
set(SC2PP_TEST_SOURCES 
    TestAgent.cpp
    TestAgent.h
    main.cpp
)

add_executable(test-bot ${SC2PP_TEST_SOURCES})
target_link_libraries(test-bot sc2pp)