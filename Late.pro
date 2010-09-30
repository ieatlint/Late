#-------------------------------------------------
#
# Project created by QtCreator 2010-09-01T15:09:53
#
#-------------------------------------------------

QT       += core gui maemo5 network

TARGET = Late
TEMPLATE = app


SOURCES += main.cpp\
        late.cpp \
    nextbus.cpp

HEADERS  += late.h \
    nextbus.h

CONFIG += mobility
MOBILITY = 

symbian {
    TARGET.UID3 = 0xed179734
    # TARGET.CAPABILITY += 
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}
