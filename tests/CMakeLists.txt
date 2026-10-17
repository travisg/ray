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

# Golden renders: ray draws a scene at 64x48 single-threaded, with any extra ray options
# given, then test_golden compares every channel against golden/<name>.pfm within a tolerance.
add_executable(test_golden test_golden.cpp)
target_link_libraries(test_golden PRIVATE raycore)

function(ray_add_golden name scene)
    add_test(NAME golden_render_${name}
        COMMAND ray --scene ${scene} -x 64 -y 48 -c 1 ${ARGN}
                -o ${CMAKE_CURRENT_BINARY_DIR}/golden-${name}.pfm
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    set_tests_properties(golden_render_${name} PROPERTIES FIXTURES_SETUP golden_${name})
    add_test(NAME golden_compare_${name}
        COMMAND test_golden ${CMAKE_CURRENT_BINARY_DIR}/golden-${name}.pfm
                ${CMAKE_CURRENT_SOURCE_DIR}/golden/${name}.pfm)
    set_tests_properties(golden_compare_${name} PROPERTIES FIXTURES_REQUIRED golden_${name})
endfunction()

ray_add_golden(default default)
ray_add_golden(spheres spheres)
ray_add_golden(triangle triangle)
ray_add_golden(cbox cbox)
ray_add_golden(dude dude)
ray_add_golden(plane-mesh plane-mesh)
ray_add_golden(csphere csphere)
ray_add_golden(blob blob)
ray_add_golden(map map)
ray_add_golden(map-blob map-blob)
ray_add_golden(cornell cornell)
ray_add_golden(map-blob-gi map-blob --gi 16 --bounces 2)
ray_add_golden(sky-gi sky --gi 16 --bounces 2)
ray_add_golden(cornell-gi cornell --gi 16 --bounces 3)
ray_add_golden(cornell-passes cornell --gi 16 --passes 4)
