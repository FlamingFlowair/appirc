TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp\
	channel.cpp\
	client.cpp\
	ensemble.cpp \
    	serveur.cpp
	
HEADERS += \
    client.h \
    serveur.h \
    ensemble.h \
    channel.h \
    err.codes.h

QMAKE_CXXFLAGS += -std=c++11
