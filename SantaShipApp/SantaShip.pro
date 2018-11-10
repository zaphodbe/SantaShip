#-------------------------------------------------
#
# Project created by QtCreator 2010-11-22T21:24:13
#
#-------------------------------------------------

QT       += core gui widgets printsupport

CONFIG   += qt debug_and_release

TARGET = SantaShip
TEMPLATE = app

#QMAKE_MAC_SDK = macosx10.10

unix: CFLAGS += -DDEFAULT_DIR="~/Pictures"
#CFLAGS += QT_NO_DEBUG_OUTPUT

include($$PWD/../qts3/qts3.pri)

SOURCES += main.cpp\
        mainwindow.cpp \
    filethumbnailprovider.cpp \
    imagelayout.cpp \
    previewwindow.cpp \
    thumbnail.cpp \
    cloudsync.cpp \
    dialogcloudsetup.cpp \
    emaildialog.cpp

HEADERS  += mainwindow.h \
    filethumbnailprovider.h \
    imagelayout.h \
    previewwindow.h \
    thumbnail.h \
    cloudsync.h \
    dialogcloudsetup.h \
    emaildialog.h

FORMS    += mainwindow.ui \
    previewwindow.ui \
    dialogcloudsetup.ui \
    emaildialog.ui

RESOURCES += \
    resources.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../SMTPEmail/release/ -lSMTPEmail
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../SMTPEmail/debug/ -lSMTPEmail
else:unix: LIBS += -L$$OUT_PWD/../SMTPEmail/ -lSMTPEmail

INCLUDEPATH += $$PWD/../SMTPEmail $$PWD/../SMTPEmail/src
DEPENDPATH += $$PWD/../SMTPEmail

DISTFILES += \
    common.pri
