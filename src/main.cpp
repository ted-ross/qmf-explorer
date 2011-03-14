
#include "main.h"

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
