add_executable(
        lhecker_bachelor_tests
        path.cpp
        path.h
        path_approx.cpp
        path_search.cpp
)
target_link_libraries(
        lhecker_bachelor_tests
        PRIVATE
        gtest_main
        lhecker_bachelor
)

add_test(NAME tests COMMAND lhecker_bachelor_tests)
