include(../set-vars.cmake)

# Put host specific functions here.  This allows us to build things like elf2cfetbl which are built using the host
# toolchain, not the cross toolchain.
build_airliner_cfe_host_tools(
    TARGET_PSP ${TARGET_PSP}
    TARGET_OSAL ${TARGET_OSAL}
    HOST_PSP ${TARGET_PSP}
    HOST_OSAL ${TARGET_OSAL}
    CONFIG
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc
    FILESYS
        /cf/apps
        /cf/download
        /cf/log
        /cf/upload
        /ram
    STARTUP_SCRIPT
        ${CMAKE_CURRENT_SOURCE_DIR}/cfe_es_startup.scr
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/cfe_mission_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/cfe_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/osconfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/cfe_platform_cfg.h
)

# Build the 'Ground Node' for CFDP file transfer.
build_airliner_cfe(
    PREFIX gndnode-
    PSP ${HOST_PSP}
    OSAL ${HOST_OSAL}
    CONFIG
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc
    FILESYS
        /cf/apps
        /cf/download
        /cf/log
        /cf/upload
        /ram
    STARTUP_SCRIPT
        ${CMAKE_CURRENT_SOURCE_DIR}/cfe_es_startup.scr
    
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/cfe_mission_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/cfe_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/osconfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/cfe_platform_cfg.h
)

add_airliner_app(
    px4lib
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/px4lib/fsw/for_build)
    
add_airliner_app(
    pblib
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/pblib/fsw/for_build)

add_airliner_app(
    sch
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/sch/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/sch
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/sch/sch_custom.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/sch_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/sch_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/sch_perfids.h
    )

add_airliner_app(
    cf
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/cf/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/cf
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cf_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cf_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/cf_perfids.h
    )

add_airliner_app(
    ci
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/ci/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/ci
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/ci/ci_custom.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/ci_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/ci_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/ci_mission_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/ci_perfids.h
    )

add_airliner_app(
    to
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/to/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/to
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/to/to_custom.c
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/to/to_custom_udp.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/to_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/to_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/to_mission_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/to_perfids.h
    )


