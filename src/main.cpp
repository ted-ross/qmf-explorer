
#include "main.h"


QmfExplorer::QmfExplorer(QMainWindow* parent) : QMainWindow(parent)
{
    setupUi(this);
    qmf = new QmfThread(this);
    qmf->start();
    connect(qmf, SIGNAL(connectionStatusChanged(QString)), label_connection_status, SLOT(setText(QString)));
    connect(actionOpen_Localhost, SIGNAL(triggered()), qmf, SLOT(connect_localhost()));
    connect(actionClose, SIGNAL(triggered()), qmf, SLOT(disconnect()));
    connect(qmf, SIGNAL(isConnected(bool)), tabWidget, SLOT(setEnabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), actionOpen_Localhost, SLOT(setDisabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), actionOpen, SLOT(setDisabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), actionClose, SLOT(setEnabled(bool)));
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
