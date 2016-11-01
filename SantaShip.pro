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
#include(SMTPEmail.pri)

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




win32:CONFIG(release, debug|release): LIBS += -L$$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_MinGW_32bit-Debug/release/ -lSMTPEmail
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_MinGW_32bit-Debug/debug/ -lSMTPEmail
else:unix: LIBS += -L$$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_clang_64bit-Debug/ -lSMTPEmail

win32:CONFIG(release, debug|release): INCLUDEPATH += $$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_MinGW_32bit-Debug/release
else:win32:CONFIG(debug, debug|release): INCLUDEPATH += $$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_MinGW_32bit-Debug/debug
else:unix: INCLUDEPATH += $$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_clang_64bit-Debug/debug

win32:CONFIG(release, debug|release): DEPENDPATH += $$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_MinGW_32bit-Debug/release
else:win32:CONFIG(debug, debug|release): DEPENDPATH += $$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_MinGW_32bit-Debug/debug
else:unix: DEPENDPATH += $$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_clang_64bit-Debug/debug

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_MinGW_32bit-Debug/release/libSMTPEmail.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_MinGW_32bit-Debug/debug/libSMTPEmail.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_MinGW_32bit-Debug/release/SMTPEmail.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_MinGW_32bit-Debug/debug/SMTPEmail.lib
else:unix: PRE_TARGETDEPS += $$PWD/build-SMTPEmail-Desktop_Qt_5_7_0_clang_64bit-Debug/libSMTPEmail.dylib
