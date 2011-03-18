#ifndef _qe_main_h
#define _qe_main_h
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

#include <QtGui>
#include <qmf/ConsoleEvent.h>
#include "ui_explorer_main.h"
#include "qmf-thread.h"
#include "agent-model.h"
#include "agent-detail-model.h"
#include "object-model.h"
#include "object-detail-model.h"

class QmfExplorer : public QMainWindow, private Ui::MainWindow {
    Q_OBJECT

public:
    QmfExplorer(QMainWindow* parent = 0);
    ~QmfExplorer();

public slots:

private:
    QmfThread* qmf;

    AgentModel* agentModel;
    AgentDetailModel* agentDetail;

    ObjectModel* objectModel;
    ObjectDetailModel* objectDetail;
};

#endif
