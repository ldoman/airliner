include(../set-vars.cmake)

# Put target specific functions here.  This is where we build most everything.  Everything that either uses the
# cross toolchain or has target independent functions.

build_airliner_cfe(
    PSP ${PSP}
    OSAL ${OSAL}
    CONFIG
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
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/osconfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cfe_platform_cfg.h
    UNIT_TEST_WRAPPER
        /home/jenkins/airliner/config/aero/default/target/unit_test_wrapper
)

add_airliner_app(
    cfs_lib
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/cfs_lib/fsw/for_build)

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
    cs
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/cs/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/cs
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cs_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cs_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/cs_mission_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/cs_perfids.h
    )

add_airliner_app(
    ds
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/ds/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/ds
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/ds_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/ds_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/ds_perfids.h
    )

add_airliner_app(
    fm
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/fm/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/fm
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/fm_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/fm_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/fm_perfids.h
    )

add_airliner_app(
    hk
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/hk/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/hk
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/hk_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/hk_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/hk_perfids.h
    )

add_airliner_app(
    hs
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/hs/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/hs
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/hs/hs_custom.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/hs_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/hs_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/hs_perfids.h
    )

add_airliner_app(
    lc
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/lc/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/lc
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/lc/src/lc_custom.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/lc_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/lc_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/lc_mission_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/lc_perfids.h
    )

add_airliner_app(
    md
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/md/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/md
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/md_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/md_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/md_perfids.h
    )

add_airliner_app(
    mm
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/mm/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/mm
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/mm_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/mm_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/mm_mission_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/mm_perfids.h
    )

add_airliner_app(
    sc
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/sc/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/sc
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/sc_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/sc_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/sc_perfids.h
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

add_airliner_app(
    ea
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/ea/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/ea
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/ea/src/ea_custom.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/ea_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/ea_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/ea_perfids.h
    )

add_airliner_app(
    vc
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/vc/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/vc
    CONFIG_SOURCES
        #${CMAKE_CURRENT_SOURCE_DIR}/apps/vc/src/vc_dev_io_udp/vc_dev_io_udp.c
        #${CMAKE_CURRENT_SOURCE_DIR}/apps/vc/src/vc_dev_io_udp/vc_dev_io_udp.h
        # device layer
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vc/src/vc_dev_io_v4l/vc_dev_io_v4l.c
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vc/src/vc_dev_io_v4l/vc_dev_io_v4l.h
        # transmit layer
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vc/src/vc_transmit_udp/vc_transmit_udp.c
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vc/src/vc_transmit_udp/vc_transmit_udp.h
        # custom utilities
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vc/src/vc_dev_utils/vc_dev_utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vc/src/vc_dev_utils/vc_dev_utils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vc/src/vc_custom.c
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vc/src/vc_custom_shared.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/vc_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/vc_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/vc_mission_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/vc_perfids.h
    )
    
set_property(TARGET vc
            APPEND_STRING
            PROPERTY
            LINK_FLAGS
            "-Wl,--no-as-needed -ljpeg"
    )

add_airliner_app(
    px4br
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/px4br/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/px4br
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/px4br_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/px4br_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/px4br_perfids.h
    )

add_airliner_app(
    amc
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/amc/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/amc
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/amc/amc_custom.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/amc_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/amc_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/amc_perfids.h
)

add_airliner_app(
    mac
    DEFINITION ${PROJECT_SOURCE_DIR}/apps/mac/fsw/for_build
    CONFIG     ${CMAKE_CURRENT_SOURCE_DIR}/apps/mac
    CONFIG_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/mac_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/mac_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared/inc/mac_perfids.h
)
