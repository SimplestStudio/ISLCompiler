TEMPLATE = app
TARGET   = islcompiler
CONFIG  += c++11 console utf8_source
CONFIG  -= qt
CONFIG  -= debug_and_release debug_and_release_target

HEADERS += \
    $$PWD/src/islparser.h \
    $$PWD/src/utils.h \
    $$PWD/src/version.h

SOURCES += \
    $$PWD/src/main.cpp \
    $$PWD/src/islparser.cpp \
    $$PWD/src/utils.cpp

win32 {
    CONFIG -= embed_manifest_exe
    RC_FILE = $$PWD/res/version.rc

    HEADERS += \
        $$PWD/src/resource.h

    OTHER_FILES += \
        $$PWD/res/version.rc \
        $$PWD/res/manifest/islcompiler.exe.manifest
}

DESTDIR = $$PWD/build
OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/rcc
