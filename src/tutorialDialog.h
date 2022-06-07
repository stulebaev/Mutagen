// tutorialDialog.h
#ifndef TUTORIALDIALOG_H
#define TUTORIALDIALOG_H

#include <QtCore/QFile>
#include "ui_tutorialDialog.h"

class TutorialDialog : public QDialog {
  Q_OBJECT
public:
  TutorialDialog() { m_ui.setupUi(this); loadPage(1); }
private slots:
  void on_buttonBack_clicked() { loadPage(m_page-1); }
  void on_buttonNext_clicked() { loadPage(m_page+1); }
private:
  int m_page;
  Ui::TutorialDialog m_ui;
  void loadPage(int page) {
    m_page = page;
    QString path = QString(":/tutorial/page%1.html").arg(m_page, 2, 10, QLatin1Char('0'));
    m_ui.textBrowser->setSource(QUrl::fromLocalFile(path));
    m_ui.buttonBack->setEnabled(m_page > 1);
    QString nextPath = QString(":/tutorial/page%1.html").arg(m_page+1, 2, 10, QLatin1Char('0'));
    m_ui.buttonNext->setEnabled(QFile::exists(nextPath));
  }
};

#endif // TUTORIALDIALOG_H
