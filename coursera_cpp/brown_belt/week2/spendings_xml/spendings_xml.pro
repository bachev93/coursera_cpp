TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    spendings_xml.cpp

unix:!macx: LIBS += -L$$OUT_PWD/../../brown_belt_lib/ -lbrown_belt_lib

INCLUDEPATH += $$PWD/../../brown_belt_lib
DEPENDPATH += $$PWD/../../brown_belt_lib

unix:!macx: LIBS += -L$$OUT_PWD/../xml_lib/ -lxml_lib

INCLUDEPATH += $$PWD/../xml_lib
DEPENDPATH += $$PWD/../xml_lib
