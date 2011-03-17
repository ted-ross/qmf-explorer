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

#include "qmf-thread.h"
#include <qpid/messaging/exceptions.h>
#include <iostream>
#include <string>

QmfThread::QmfThread(QObject* parent, AgentModel *agents) :
    QThread(parent), cancelled(false), connected(false), agentModel(agents)
{
}


void QmfThread::cancel()
{
    cancelled = true;
}


void QmfThread::connect_localhost()
{
    QMutexLocker locker(&lock);
    command_queue.push_back(Command(true, "localhost", "", ""));
    cond.wakeOne();
}


void QmfThread::connect(const std::string& url, const std::string& conn_options, const std::string& qmf_options)
{
    QMutexLocker locker(&lock);
    command_queue.push_back(Command(true, url, conn_options, qmf_options));
    cond.wakeOne();
}


void QmfThread::disconnect()
{
    QMutexLocker locker(&lock);
    command_queue.push_back(Command(false, "", "", ""));
    cond.wakeOne();
}


void QmfThread::setupAgents()
{
}


void QmfThread::handleAgentAdd(const qmf::ConsoleEvent& event)
{
    //agentModel->addAgent(event.getAgent());
    emit newAgent(event.getAgent());
}


void QmfThread::handleAgentDel(const qmf::ConsoleEvent& event)
{
}


void QmfThread::run()
{
    emit connectionStatusChanged("Closed");
    setupAgents();

    while(true) {
        if (connected) {
            qmf::ConsoleEvent event;
            if (sess.nextEvent(event, qpid::messaging::Duration::SECOND)) {
                //
                // Process the event
                //
                switch (event.getType()) {
                case qmf::CONSOLE_AGENT_ADD : handleAgentAdd(event); break;
                case qmf::CONSOLE_AGENT_DEL : handleAgentDel(event); break;
                default: break;
                }
            }

            {
                QMutexLocker locker(&lock);
                if (command_queue.size() > 0) {
                    Command command(command_queue.front());
                    command_queue.pop_front();
                    if (!command.connect) {
                        emit connectionStatusChanged("QMF Session Closing...");
                        sess.close();
                        emit connectionStatusChanged("Closing...");
                        conn.close();
                        emit connectionStatusChanged("Closed");
                        connected = false;
                        emit isConnected(false);
                    }
                }
            }
        } else {
            QMutexLocker locker(&lock);
            if (command_queue.size() == 0)
                cond.wait(&lock, 1000);
            if (command_queue.size() > 0) {
                Command command(command_queue.front());
                command_queue.pop_front();
                if (command.connect & !connected)
                    try {
                        conn = qpid::messaging::Connection(command.url, command.conn_options);
                        conn.open();
                        sess = qmf::ConsoleSession(conn, command.qmf_options);
                        sess.open();
                        sess.setAgentFilter("[]");
                        connected = true;
                        emit isConnected(true);

                        std::stringstream line;
                        line << "Operational (URL: " << command.url << ")";
                        emit connectionStatusChanged(line.str().c_str());
                    } catch(qpid::messaging::MessagingException& ex) {
                        std::stringstream line;
                        line << "QMF Session Failed: " << ex.what();
                        emit connectionStatusChanged(line.str().c_str());
                    }
            }
        }

        if (cancelled) {
            if (connected) {
                sess.close();
                conn.close();
            }
            break;
        }
    }
}

