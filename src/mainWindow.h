// mainWindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QUndoStack>
#include <QtCore/QSettings>
#include "scene3D.h"
#include "tutorialDialog.h"

#define LOGFILENAME  "Mutagen.log"
#define INIFILENAME  "Mutagen.ini"


class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow();
  ~MainWindow();
private slots:
  void open();
  void save();
  void cut();
  void copy();
  void paste();
  void deleteSlot();
  void undo();
  void redo();
  void changeWindowState();
  void help();
  void about();
private:
  void readSettings();
  void writeSettings();
  void createActions();
  void createMenus();
  void createToolBar();
private:
  QMenu* fileMenu;
  QMenu* editMenu;
  QMenu* displayMenu;
  QMenu* helpMenu;
  QToolBar* toolBar;
  QAction* openAct;
  QAction* saveAct;
  QAction* quitAct;
  QAction* undoAct;
  QAction* redoAct;
  QAction* cutAct;
  QAction* copyAct;
  QAction* pasteAct;
  QAction* deleteAct;
  QAction* changeWindowStateAct;
  QAction* helpAct;
  QAction* aboutAct;
private:
  QSettings* settings;
  Scene3D* sceneView;
  QUndoStack* undoStack;
  TutorialDialog* tutorialDialog;
};

#endif // MAINWINDOW_H
