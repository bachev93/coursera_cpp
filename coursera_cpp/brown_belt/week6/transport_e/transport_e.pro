TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp \
    descriptions.cpp \
    json.cpp \
    requests.cpp \
    sphere.cpp \
    transport_catalog.cpp \
    transport_router.cpp \
    utils.cpp

HEADERS += \
    descriptions.h \
    graph.h \
    json.h \
    requests.h \
    router.h \
    sphere.h \
    transport_catalog.h \
    transport_router.h \
    utils.h

unix:!macx: LIBS += -L$$OUT_PWD/../../brown_belt_lib/ -lbrown_belt_lib

INCLUDEPATH += $$PWD/../../brown_belt_lib
DEPENDPATH += $$PWD/../../brown_belt_lib
