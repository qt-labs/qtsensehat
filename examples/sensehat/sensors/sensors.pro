QT += sensehat
CONFIG += c++11

SOURCES = main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/sensehat/sensors
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS sensors.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/sensehat/sensors
INSTALLS += target sources
