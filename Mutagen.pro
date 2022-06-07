TEMPLATE = app
TARGET = Mutagen

QT += opengl xml
CONFIG += release

DESTDIR = ./
MOC_DIR = ./tmp
RCC_DIR = ./tmp
UI_DIR = ./tmp
OBJECTS_DIR = ./tmp
INCLUDEPATH += ./src

HEADERS = \
  src/mainWindow.h \
  src/scene3D.h \
  src/tutorialDialog.h

SOURCES = \
  src/main.cpp \
  src/mainWindow.cpp \
  src/scene3D.cpp \
  src/molecular.cpp \
  src/mapping.cpp \
  src/CommonLib/Exception.cpp \
  src/CommonLib/Logger.cpp \
  src/CommonLib/PeriodicTable.cpp \
  src/FileIOLib/FileIOBase.cpp

FORMS = \
  src/tutorialDialog.ui

RESOURCES = \
  src/icons.qrc \
  src/tutorial.qrc

win32 {
  LIBS += opengl32.lib
}
