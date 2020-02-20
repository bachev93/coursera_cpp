TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    matrix_sum.cpp

QMAKE_CXXFLAGS += -pthread
LIBS += -pthread

unix:!macx: LIBS += -L$$OUT_PWD/../../red_belt_lib/ -lred_belt_lib

INCLUDEPATH += $$PWD/../../red_belt_lib
DEPENDPATH += $$PWD/../../red_belt_lib

HEADERS += \
    paginator.h
