TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp\
	channel.cpp\
	client.cpp\
	serveur.cpp
	
HEADERS += \
    client.h \
    serveur.h \
    channel.h \
    codes.h

QMAKE_CXXFLAGS += -std=c++0x
