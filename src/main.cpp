
#include "main.h"


QmfExplorer::QmfExplorer(QMainWindow* parent) : QMainWindow(parent)
{
    setupUi(this);
    qmf = new QmfThread(this);
    qmf->start();
    connect(qmf, SIGNAL(connectionStatusChanged(QString)), label_connection_status, SLOT(setText(QString)));
    connect(actionOpen_Localhost, SIGNAL(triggered()), qmf, SLOT(connect_localhost()));
    connect(actionClose, SIGNAL(triggered()), qmf, SLOT(disconnect()));
}


QmfExplorer::~QmfExplorer()
{
    qmf->cancel();
    qmf->wait();
    delete qmf;
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow *window = new QMainWindow;
    QmfExplorer qe(window);

    qe.show();
    return app.exec();
}
