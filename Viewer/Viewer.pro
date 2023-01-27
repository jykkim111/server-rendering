# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# ------------------------------------------------------

TEMPLATE = app
TARGET = Viewer
DESTDIR = ../x64/Debug
QT += core widgets
CONFIG += debug console
LIBS += -L"$(SolutionDir)/3rdparty/spdlog-1.8.1/lib/$(Platform)/$(Configuration)" \
    -L"$(SolutionDir)build/lib/$(Platform)/$(Configuration)" \
    -L"$(Qt_LIBPATH_)" \
    -lViewerManager \
    -lCgipCommon \
    -lCgipException \
    -lCgipIO \
    -lspdlogd \
    -l$(Qt_LIBS_)
DEPENDPATH += .
MOC_DIR += .
OBJECTS_DIR += debug
UI_DIR += .
RCC_DIR += .
include(Viewer.pri)
