set(APP_NAME to)

add_airliner_app_table(
    ${APP_NAME}
    NAME to_grnd_bin
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tables/to_ground_binary.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

add_airliner_app_table(
    ${APP_NAME}
    NAME to_grnd_pb
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tables/to_ground_protobuf.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

add_airliner_app_table(
    ${APP_NAME}
    NAME to_grnd_cfdp
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tables/to_ground_cfdp.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

#add_airliner_app_unit_test(to-custom-ut
#    UTASSERT
#    SOURCES
#        ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/to_app_custom_test.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/to_app_custom_test.h
#        ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/to_custom_testrunner.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/to_custom_test_utils.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/to_custom_test_utils.h
#        ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/to_app_stubs.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/to_app_stubs.h
#        ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/to_platform_stubs.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/to_platform_stubs.h
#        ${CMAKE_CURRENT_SOURCE_DIR}/to_custom.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/to_custom_udp.h
#
#    INCLUDES
#        ${CMAKE_CURRENT_SOURCE_DIR}/
#        ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/
#        ${PROJECT_SOURCE_DIR}/apps/to/fsw/src
#        
#    WRAPPERS
#        socket
#        bind
#        sendto
#)
