
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_samples
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(test_transpiler test_transpiler.cpp)
target_link_libraries(test_transpiler PRIVATE indentationisoptional_core)
add_test(NAME TranspilerTests COMMAND test_transpiler)

add_executable(test_checker test_checker.cpp)
target_link_libraries(test_checker PRIVATE indentationisoptional_core)
add_test(NAME CheckerTests COMMAND test_checker)
