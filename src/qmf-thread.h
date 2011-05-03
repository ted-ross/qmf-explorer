#ifndef _qe_qmf_thread_h
#define _qe_qmf_thread_h
/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QLineEdit>
#include <qpid/messaging/Connection.h>
#include <qmf/ConsoleSession.h>
#include <qmf/ConsoleEvent.h>
#include "agent-model.h"
#include "object-model.h"
#include <sstream>
#include <deque>

class QmfThread : public QThread {
    Q_OBJECT

public:
    QmfThread(QObject* parent, AgentModel* agents, QLineEdit* agentFilter, ObjectModel* objects);
    void cancel();
    void connect(const std::string& url, const std::string& conn_options, const std::string& qmf_options);

public slots:
    void connect_localhost();
    void disconnect();
    void applyAgentFilter();

signals:
    void connectionStatusChanged(const QString&);
    void isConnected(bool);
    void newAgent(const qmf::Agent&);
    void delAgent(const qmf::Agent&);
    void newPackage(const QString&);

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

    AgentModel* agentModel;
    QLineEdit* agentFilter;
    ObjectModel* objectModel;
};

#endif

