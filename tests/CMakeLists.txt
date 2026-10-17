# Each test is a plain executable that exits non-zero on failure (see check.h).
function(ray_add_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE raycore)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

ray_add_test(test_vector3)
ray_add_test(test_rayfile)
ray_add_test(test_objloader)
target_compile_definitions(test_objloader PRIVATE RAY_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

# Golden renders: ray draws a scene at 64x48 single-threaded, then test_golden
# compares every channel against the reference in golden/ within a tolerance.
add_executable(test_golden test_golden.cpp)
target_link_libraries(test_golden PRIVATE rayfile)

function(ray_add_golden scene)
    add_test(NAME golden_render_${scene}
        COMMAND ray -x 64 -y 48 -c 1 -o ${CMAKE_CURRENT_BINARY_DIR}/golden-${scene}.ray
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    set_tests_properties(golden_render_${scene} PROPERTIES FIXTURES_SETUP golden_${scene})
    add_test(NAME golden_compare_${scene}
        COMMAND test_golden ${CMAKE_CURRENT_BINARY_DIR}/golden-${scene}.ray
                ${CMAKE_CURRENT_SOURCE_DIR}/golden/${scene}.ray)
    set_tests_properties(golden_compare_${scene} PROPERTIES FIXTURES_REQUIRED golden_${scene})
endfunction()

ray_add_golden(default)
