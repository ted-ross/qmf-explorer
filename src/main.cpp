
#include <QtGui>
#include "ui_explorer_main.h"

class QmfExplorer : public QMainWindow {
    //Q_OBJECT

  public:
    QmfExplorer(QMainWindow* parent = 0);
    virtual ~QmfExplorer() {}

    //  private slots:

  private:
    Ui::MainWindow ui;
};

QmfExplorer::QmfExplorer(QMainWindow* parent) : QMainWindow(parent)
{
    ui.setupUi(this);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow *window = new QMainWindow;
    QmfExplorer qe(window);

    qe.show();
    return app.exec();
}
