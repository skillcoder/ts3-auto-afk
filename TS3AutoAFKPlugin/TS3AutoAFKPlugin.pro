# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# ------------------------------------------------------

TEMPLATE = lib
TARGET = TS3AutoAFKPlugin
DESTDIR = ../x64/Release
QT += core gui widgets
CONFIG += debug
DEFINES += _WINDOWS _USRDLL TS3AUTOAFKPLUGIN_EXPORTS
INCLUDEPATH += ./include \
    $(NOINHERIT)
LIBS += -L"."
DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/$(ConfigurationName)
OBJECTS_DIR += release
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles
include(TS3AutoAFKPlugin.pri)
