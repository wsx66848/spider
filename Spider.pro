TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    http.c \
    bloom.c \
    hash.c \
    queue.c \
    parser.c \
    arraylist.c \
    log.c \
    map.c

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libevent/lib/ -llibevent -llibevent_core -llibevent_extras
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libevent/lib/ -llibevent -llibevent_core -llibevent_extras
else:unix: LIBS += -L$$PWD/libevent/lib/ -llibevent -llibevent_core -llibevent_extras

INCLUDEPATH += $$PWD/libevent/include
DEPENDPATH += $$PWD/libevent/include
LIBS += libwsock32 libWs2_32 -lpthread

HEADERS += \
    http.h \
    common.h \
    bloom.h \
    hash.h \
    queue.h \
    parser.h \
    arraylist.h \
    log.h \
    map.h
