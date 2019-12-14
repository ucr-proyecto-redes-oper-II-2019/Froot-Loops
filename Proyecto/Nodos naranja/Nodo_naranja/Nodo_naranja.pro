TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS+= -fopenmp
QMAKE_LFLAGS +=  -fopenmp

SOURCES += \
        Nodo_naranja.cpp \
        main.cpp

HEADERS += \
    Nodo_naranja.h
