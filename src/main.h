#ifndef _qe_main_h
#define _qe_main_h

#include <QtGui>
#include "ui_explorer_main.h"
#include "qmf-thread.h"

class QmfExplorer : public QMainWindow, private Ui::MainWindow {
    Q_OBJECT

  public:
    QmfExplorer(QMainWindow* parent = 0);
    ~QmfExplorer();

  public slots:

  private:
    QmfThread* qmf;
};

#endif
