DESTDIR = ..
TARGET = write_plugins_project_app
TEMPLATE = app

QT       -= gui
CONFIG   -= app_bundle
CONFIG   += console
CONFIG   += release

# Load application icon
RC_FILE = ../../../resources/unisim.rc

HEADERS += \
    ../common/common.h

SOURCES += \
    ../common/common.cpp \
    main.cpp
