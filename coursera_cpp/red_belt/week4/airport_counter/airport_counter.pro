TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -O2

SOURCES += \
    airport_counter.cpp

HEADERS += \
    test_runner.h \
    profile.h
