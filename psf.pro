HEADERS += decoderpsffactory.h \
           decoder_psf.h \
           psfhelper.h \
           psfmetadatamodel.h

SOURCES += decoderpsffactory.cpp \
           decoder_psf.cpp \
           psfhelper.cpp \
           psfmetadatamodel.cpp \
           libpsf/corlett.c \
           libpsf/ao.c \
           libpsf/eng_dsf/eng_dsf.c \
           libpsf/eng_dsf/dc_hw.c \
           libpsf/eng_dsf/aica.c \
           libpsf/eng_dsf/aicadsp.c \
           libpsf/eng_dsf/arm7.c \
           libpsf/eng_dsf/arm7i.c \
           libpsf/eng_ssf/m68kcpu.c \
           libpsf/eng_ssf/m68kopac.c \
           libpsf/eng_ssf/m68kopdm.c \
           libpsf/eng_ssf/m68kopnz.c \
           libpsf/eng_ssf/m68kops.c \
           libpsf/eng_ssf/scsp.c \
           libpsf/eng_ssf/scspdsp.c \
           libpsf/eng_ssf/sat_hw.c \
           libpsf/eng_ssf/eng_ssf.c \
           libpsf/eng_qsf/eng_qsf.c \
           libpsf/eng_qsf/kabuki.c \
           libpsf/eng_qsf/qsound.c \
           libpsf/eng_qsf/z80.c \
           libpsf/eng_qsf/z80dasm.c \
           libpsf/eng_psf/eng_psf.c \
           libpsf/eng_psf/psx.c \
           libpsf/eng_psf/psx_hw.c \
           libpsf/eng_psf/peops/spu.c \
           libpsf/eng_psf/eng_psf2.c \
           libpsf/eng_psf/peops2/spu2.c \
           libpsf/eng_psf/peops2/dma2.c \
           libpsf/eng_psf/peops2/registers2.c \
           libpsf/eng_psf/eng_spu.c

SOURCES += libpsf/zlib/adler32.c \
           libpsf/zlib/compress.c \
           libpsf/zlib/crc32.c \
           libpsf/zlib/deflate.c \
           libpsf/zlib/gzclose.c \
           libpsf/zlib/gzlib.c \
           libpsf/zlib/gzread.c \
           libpsf/zlib/gzwrite.c \
           libpsf/zlib/inflate.c \
           libpsf/zlib/infback.c \
           libpsf/zlib/inftrees.c \
           libpsf/zlib/inffast.c \
           libpsf/zlib/trees.c \
           libpsf/zlib/uncompr.c \
           libpsf/zlib/zutil.c

INCLUDEPATH += $$PWD/libpsf

#CONFIG += BUILD_PLUGIN_INSIDE
contains(CONFIG, BUILD_PLUGIN_INSIDE){
    include($$PWD/../../plugins.pri)
    TARGET = $$PLUGINS_PREFIX/Input/psf

    unix{
        target.path = $$PLUGIN_DIR/Input
        INSTALLS += target
    }
}else{
    CONFIG += warn_off plugin lib thread link_pkgconfig c++11
    TEMPLATE = lib

    unix{
        equals(QT_MAJOR_VERSION, 5){
            QMMP_PKG = qmmp-1
        }else:equals(QT_MAJOR_VERSION, 6){
            QMMP_PKG = qmmp
        }else{
            error("Unsupported Qt version: 5 or 6 is required")
        }
        
        PKGCONFIG += $${QMMP_PKG}

        PLUGIN_DIR = $$system(pkg-config $${QMMP_PKG} --variable=plugindir)/Input
        INCLUDEPATH += $$system(pkg-config $${QMMP_PKG} --variable=prefix)/include

        plugin.path = $${PLUGIN_DIR}
        plugin.files = lib$${TARGET}.so
        INSTALLS += plugin
    }
}
