#-------------------------------------------------
#
# Project created by QtCreator 2010-11-22T21:24:13
#
#-------------------------------------------------

QT       += core gui

TARGET = SantaShip
TEMPLATE = app

unix: CFLAGS += -DDEFAULT_DIR="~/Pictures"

SOURCES += main.cpp\
        mainwindow.cpp \
    filethumbnailprovider.cpp \
    imagelayout.cpp \
    previewwindow.cpp

HEADERS  += mainwindow.h \
    filethumbnailprovider.h \
    imagelayout.h \
    previewwindow.h

FORMS    += mainwindow.ui \
    previewwindow.ui



