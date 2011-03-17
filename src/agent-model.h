#ifndef _qe_agent_model_h
#define _qe_agent_model_h
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

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QMutex>
#include <qmf/Agent.h>
#include <sstream>
#include <string>
#include <map>
#include <list>
#include <deque>
#include <boost/shared_ptr.hpp>

Q_DECLARE_METATYPE(qmf::Agent);

class AgentModel : public QAbstractItemModel {
    Q_OBJECT

public:
    AgentModel(QObject* parent = 0);


    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex parent(const QModelIndex& index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

public slots:
    void addAgent(const qmf::Agent&);
    void delAgent(const qmf::Agent&);

private:
    typedef enum { NODE_VENDOR, NODE_PRODUCT } NodeType;
    struct AgentIndex;
    typedef boost::shared_ptr<AgentIndex> AgentIndexPtr;
    typedef std::map<quint32, AgentIndexPtr> IndexMap;
    typedef std::list<AgentIndexPtr> IndexList;
    typedef std::list<qmf::Agent> AgentList;

    struct AgentIndex {
        quint32 id;
        int row;
        int column;
        NodeType nodeType;
        std::string text;
        AgentIndexPtr parent;
        IndexList children;
        AgentList agents;
    };

    IndexList vendors;
    IndexMap linkage;
    quint32 nextId;
};

#endif

