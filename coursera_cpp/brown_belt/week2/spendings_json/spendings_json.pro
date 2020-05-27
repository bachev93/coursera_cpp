TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    spendings_json.cpp

unix:!macx: LIBS += -L$$OUT_PWD/../../brown_belt_lib/ -lbrown_belt_lib

INCLUDEPATH += $$PWD/../../brown_belt_lib
DEPENDPATH += $$PWD/../../brown_belt_lib

unix:!macx: LIBS += -L$$OUT_PWD/../json_lib/ -ljson_lib

INCLUDEPATH += $$PWD/../json_lib
DEPENDPATH += $$PWD/../json_lib
