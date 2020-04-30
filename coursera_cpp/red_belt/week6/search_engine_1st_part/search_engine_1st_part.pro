TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt

unix:!macx: LIBS += -L$$OUT_PWD/../../red_belt_lib/ -lred_belt_lib

INCLUDEPATH += $$PWD/../../red_belt_lib
DEPENDPATH += $$PWD/../../red_belt_lib

SOURCES += \
    main.cpp \
    search_server.cpp \
    parse.cpp

HEADERS += \
    total_duration.h \
    search_server.h \
    parse.h \
    iterator_range.h
