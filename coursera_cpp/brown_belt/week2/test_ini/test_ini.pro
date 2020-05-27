TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    test_ini.cpp

unix:!macx: LIBS += -L$$OUT_PWD/../../brown_belt_lib/ -lbrown_belt_lib

INCLUDEPATH += $$PWD/../../brown_belt_lib
DEPENDPATH += $$PWD/../../brown_belt_lib

unix:!macx: LIBS += -L$$OUT_PWD/../ini_lib/ -lini_lib

INCLUDEPATH += $$PWD/../ini_lib
DEPENDPATH += $$PWD/../ini_lib
