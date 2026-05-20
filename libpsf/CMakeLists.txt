cmake_minimum_required(VERSION 3.18.0)

project(psf)

add_definitions(-DHAS_PSXCPU)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(HEADERS
    ao.h
)

set(SOURCES
    corlett.c
    ao.c
    eng_dsf/eng_dsf.c
    eng_dsf/dc_hw.c
    eng_dsf/aica.c
    eng_dsf/aicadsp.c
    eng_dsf/arm7.c
    eng_dsf/arm7i.c
    eng_ssf/m68kcpu.c
    eng_ssf/m68kopac.c
    eng_ssf/m68kopdm.c
    eng_ssf/m68kopnz.c
    eng_ssf/m68kops.c
    eng_ssf/scsp.c
    eng_ssf/scspdsp.c
    eng_ssf/sat_hw.c
    eng_ssf/eng_ssf.c
    eng_qsf/eng_qsf.c
    eng_qsf/kabuki.c
    eng_qsf/qsound.c
    eng_qsf/z80.c
    eng_qsf/z80dasm.c
    eng_psf/eng_psf.c
    eng_psf/psx.c
    eng_psf/psx_hw.c
    eng_psf/peops/spu.c
    eng_psf/eng_psf2.c
    eng_psf/peops2/spu2.c
    eng_psf/peops2/dma2.c
    eng_psf/peops2/registers2.c
    eng_psf/eng_spu.c
)

set(ZLIB_SOURCES
    zlib/adler32.c
    zlib/compress.c
    zlib/crc32.c
    zlib/deflate.c
    zlib/gzclose.c
    zlib/gzlib.c
    zlib/gzread.c
    zlib/gzwrite.c
    zlib/inflate.c
    zlib/infback.c
    zlib/inftrees.c
    zlib/inffast.c
    zlib/trees.c
    zlib/uncompr.c
    zlib/zutil.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS} ${ZLIB_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}${STATIC_LIB_SUFFIX})

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    COMPONENT library
)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lib${PROJECT_NAME})
