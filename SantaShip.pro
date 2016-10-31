#-------------------------------------------------
#
# Project created by QtCreator 2010-11-22T21:24:13
#
#-------------------------------------------------

QT       += core gui widgets printsupport

CONFIG   += qt debug

TARGET = SantaShip
TEMPLATE = app

#QMAKE_MAC_SDK = macosx10.10

unix: CFLAGS += -DDEFAULT_DIR="~/Pictures"
#CFLAGS += QT_NO_DEBUG_OUTPUT

include(qts3/qts3.pri)
include(SMTPEmail.pri)

SOURCES += main.cpp\
        mainwindow.cpp \
    filethumbnailprovider.cpp \
    imagelayout.cpp \
    previewwindow.cpp \
    thumbnail.cpp \
    cloudsync.cpp \
    dialogcloudsetup.cpp

HEADERS  += mainwindow.h \
    filethumbnailprovider.h \
    imagelayout.h \
    previewwindow.h \
    thumbnail.h \
    cloudsync.h \
    dialogcloudsetup.h

FORMS    += mainwindow.ui \
    previewwindow.ui \
    dialogcloudsetup.ui

RESOURCES += \
    resources.qrc



