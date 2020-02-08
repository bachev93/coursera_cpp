TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    booking.cpp

HEADERS +=

unix:!macx: LIBS += -L$$OUT_PWD/../../red_belt_lib/ -lred_belt_lib

INCLUDEPATH += $$PWD/../../red_belt_lib
DEPENDPATH += $$PWD/../../red_belt_lib
