win32{
	PLATEFORM = windows
}

unix{
	PLATEFORM = linux
}

#Sets target destination dir - platform independent
CONFIG(debug, debug|release) {
    CONFIG -= console
    DLLDESTDIR = $$PWD/build/$$PLATEFORM/debug/lib
    DESTDIR = $$PWD/build/$$PLATEFORM/debug/bin
}

CONFIG(release, debug|release) {
    CONFIG -= console
    DLLDESTDIR = $$PWD/build/$$PLATEFORM/release/lib
    DESTDIR = $$PWD/build/$$PLATEFORM/release/bin
}

    QMAKE_CXXFLAGS += -std=c++11

#Includes common configuration for all subdirectory .pro files.
INCLUDEPATH += . .. \ 
            $$PWD/src/com \
            $$PWD/src/tools \


INCLUDEPATH += . ..
WARNINGS += -Wall

TEMPLATE = lib

# The following keeps the generated files at least somewhat separate
# from the source files.
UI_DIR = uics
MOC_DIR = mocs
OBJECTS_DIR = objs

