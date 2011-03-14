#ifndef _qe_main_h
#define _qe_main_h

#include <QtGui>
#include "ui_explorer_main.h"

class QmfExplorer : public QMainWindow, private Ui::MainWindow {
    Q_OBJECT

  public:
    QmfExplorer(QMainWindow* parent = 0);

  private slots:
};

#endif
