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

#include "main.h"
#include <iostream>

QmfExplorer::QmfExplorer(QMainWindow* parent) : QMainWindow(parent)
{
    setupUi(this);

    agentModel = new AgentModel(this);
    treeView_agents->setModel(agentModel);

    qmf = new QmfThread(this, agentModel);
    qmf->start();
    connect(qmf, SIGNAL(connectionStatusChanged(QString)), label_connection_status, SLOT(setText(QString)));
    connect(actionOpen_Localhost, SIGNAL(triggered()), qmf, SLOT(connect_localhost()));
    connect(actionClose, SIGNAL(triggered()), qmf, SLOT(disconnect()));

    //
    // Create linkages to enable and disable main-window components based on the connection status.
    //
    connect(qmf, SIGNAL(isConnected(bool)), tabWidget, SLOT(setEnabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), actionOpen_Localhost, SLOT(setDisabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), actionOpen, SLOT(setDisabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), actionClose, SLOT(setEnabled(bool)));

    connect(qmf, SIGNAL(newAgent()), agentModel, SLOT(safeAddAgent()));
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
