
#include <QtGui>
#include "ui_explorer_main.h"

class QmfExplorer : public QMainWindow, private Ui::MainWindow {
    //Q_OBJECT

  public:
    QmfExplorer(QMainWindow* parent = 0);

    //  private slots:
};

QmfExplorer::QmfExplorer(QMainWindow* parent) : QMainWindow(parent)
{
    setupUi(this);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow *window = new QMainWindow;
    QmfExplorer qe(window);

    qe.show();
    return app.exec();
}
