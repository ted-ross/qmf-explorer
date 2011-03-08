
#include <QtGui>

class QmfExplorer {
public:
    
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QWidget window;
    window.resize(750, 800);
    window.show();
    window.setWindowTitle(QApplication::translate("qmf", "QMF Explorer"));
    return app.exec();
}
