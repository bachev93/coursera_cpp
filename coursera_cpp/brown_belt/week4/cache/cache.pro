TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    Solution.cpp

HEADERS += \
    Common.h

QMAKE_CXXFLAGS += -pthread
LIBS += -pthread

unix:!macx: LIBS += -L$$OUT_PWD/../../brown_belt_lib/ -lbrown_belt_lib

INCLUDEPATH += $$PWD/../../brown_belt_lib
DEPENDPATH += $$PWD/../../brown_belt_lib
