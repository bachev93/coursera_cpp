TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    spendings_xml.cpp

unix:!macx: LIBS += -L$$OUT_PWD/../../brown_belt_lib/ -lbrown_belt_lib

INCLUDEPATH += $$PWD/../../brown_belt_lib
DEPENDPATH += $$PWD/../../brown_belt_lib

unix:!macx: LIBS += -L$$OUT_PWD/../week2_brown_belt_lib/ -lweek2_brown_belt_lib

INCLUDEPATH += $$PWD/../week2_brown_belt_lib
DEPENDPATH += $$PWD/../week2_brown_belt_lib
