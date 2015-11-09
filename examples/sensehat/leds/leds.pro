QT += sensehat

SOURCES = main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/sensehat/leds
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS leds.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/sensehat/leds
INSTALLS += target sources
