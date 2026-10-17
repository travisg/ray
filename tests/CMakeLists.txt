# Each test is a plain executable that exits non-zero on failure (see check.h).
function(ray_add_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE raycore)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

ray_add_test(test_vector3)
ray_add_test(test_intersect)
ray_add_test(test_bvh)
ray_add_test(test_tracer)
ray_add_test(test_pfm)
ray_add_test(test_png)
ray_add_test(test_tev)
ray_add_test(test_objloader)
target_compile_definitions(test_objloader PRIVATE RAY_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

# Golden renders: ray draws a scene at 64x48 single-threaded, then test_golden
# compares every channel against the reference in golden/ within a tolerance.
add_executable(test_golden test_golden.cpp)
target_link_libraries(test_golden PRIVATE raycore)

function(ray_add_golden scene)
    add_test(NAME golden_render_${scene}
        COMMAND ray --scene ${scene} -x 64 -y 48 -c 1
                -o ${CMAKE_CURRENT_BINARY_DIR}/golden-${scene}.pfm
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    set_tests_properties(golden_render_${scene} PROPERTIES FIXTURES_SETUP golden_${scene})
    add_test(NAME golden_compare_${scene}
        COMMAND test_golden ${CMAKE_CURRENT_BINARY_DIR}/golden-${scene}.pfm
                ${CMAKE_CURRENT_SOURCE_DIR}/golden/${scene}.pfm)
    set_tests_properties(golden_compare_${scene} PROPERTIES FIXTURES_REQUIRED golden_${scene})
endfunction()

ray_add_golden(default)
ray_add_golden(spheres)
ray_add_golden(triangle)
ray_add_golden(cbox)
ray_add_golden(dude)
ray_add_golden(plane-mesh)
ray_add_golden(csphere)
ray_add_golden(blob)
