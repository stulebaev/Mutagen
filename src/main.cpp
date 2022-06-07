#include <QApplication>
#include "mainWindow.h"

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  app.setApplicationName("Mutagen");
  app.setWindowIcon(QIcon(":/images/molecule.png"));
  MainWindow mainWindow;
  mainWindow.show();
  return app.exec();
}
