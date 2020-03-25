TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -pthread
LIBS += -pthread

unix:!macx: LIBS += -L$$OUT_PWD/../../red_belt_lib/ -lred_belt_lib

INCLUDEPATH += $$PWD/../../red_belt_lib
DEPENDPATH += $$PWD/../../red_belt_lib

SOURCES += \
    main.cpp \
    parse.cpp \
    search_server.cpp

HEADERS += \
    iterator_range.h \
    parse.h \
    search_server.h \
    total_duration.h
