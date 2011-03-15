#ifndef _qe_qmf_thread_h
#define _qe_qmf_thread_h

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <qpid/messaging/Connection.h>
#include <qmf/ConsoleSession.h>
#include <sstream>
#include <deque>

class QmfThread : public QThread {
    Q_OBJECT

public:
    QmfThread(QObject* parent);
    void cancel();
    void connect(const std::string& url, const std::string& conn_options, const std::string& qmf_options);

public slots:
    void connect_localhost();
    void disconnect();

signals:
    void connectionStatusChanged(const QString& text);
    void isConnected(bool);

protected:
    void run();

private:
    struct Command {
        bool connect;
        std::string url;
        std::string conn_options;
        std::string qmf_options;

        Command(bool _c, const std::string& _u, const std::string& _co, const std::string& _qo) :
            connect(_c), url(_u), conn_options(_co), qmf_options(_qo) {}
    };
    typedef std::deque<Command> command_queue_t;

    mutable QMutex lock;
    QWaitCondition cond;
    qpid::messaging::Connection conn;
    qmf::ConsoleSession sess;
    bool cancelled;
    bool connected;
    command_queue_t command_queue;
};

#endif

