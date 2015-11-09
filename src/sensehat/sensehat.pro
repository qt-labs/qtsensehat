TARGET = QtSenseHat
MODULE = sensehat
load(qt_module)

QT += core-private
CONFIG += c++11
DEFINES += QSENSEHAT_BUILD_LIB

SOURCES = qsensehatfb.cpp \
          qsensehatsensors.cpp

HEADERS = qsensehatfb.h \
          qsensehatsensors.h \
          qsenseglobal.h

LIBS += -lRTIMULib
