#Sets target destination dir - platform independent
# Check if the config file exists
! include( ../common.pri ) {
	error( "Couldn't find the common.pri file!" )
}

#QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp

message("The project contains the following files:")
message($$DLLDESTDIR)

LIBS += -L$$DESTDIR -L$$DLLDESTDIR \ 
        -ltools \
        -lcom \


# Will build the final executable in the main project directory.

TARGET =device_nfc_reader

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui
