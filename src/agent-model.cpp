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

#include "agent-model.h"
#include <iostream>

using std::cout;
using std::endl;

AgentModel::AgentModel(QObject* parent) : QAbstractItemModel(parent), nextId(1)
{
    // Intentionally Left Blank
}


void AgentModel::addAgent(const qmf::Agent& agent)
{
    QMutexLocker locker(&lock);
    addAgentQueue.push_back(agent);
}


void AgentModel::safeAddAgent()
{
    qmf::Agent agent;
    {
        QMutexLocker locker(&lock);
        agent = addAgentQueue.front();
        addAgentQueue.pop_front();
    }

    int rowCount;
    const std::string& vendor(agent.getVendor());
    const std::string& product(agent.getProduct());
    const std::string& instance(agent.getInstance());

    IndexList::iterator iter(vendors.begin());
    rowCount = 0;
    while (iter != vendors.end() && vendor > (*iter)->text) {
        iter++;
        rowCount++;
    }

    if (iter == vendors.end() || vendor != (*iter)->text) {
        cout << "[AgentModel::addAgent] Inserting Vendor '" << vendor << "' at " << rowCount << endl;
        //
        // We need to insert a new vendor row into the model
        //
        beginInsertRows(QModelIndex(), rowCount, rowCount);

        //
        // Allocate a new AgentIndex
        //
        AgentIndexPtr ai(new AgentIndex());
        ai->id = nextId++;
        ai->nodeType = NODE_VENDOR;
        ai->text = vendor;
        linkage[ai->id] = ai;

        if (iter == vendors.end())
            vendors.push_back(ai);
        else
            vendors.insert(iter, ai);
        endInsertRows();
    }

    cout << "Vendor List:" << endl;
    for (iter = vendors.begin(); iter != vendors.end(); iter++)
        cout << "    " << (*iter)->text << " id:" << (*iter)-> id << endl;
}


void AgentModel::delAgent(const qmf::Agent&)
{
}


int AgentModel::rowCount(const QModelIndex &parent) const
{
    QMutexLocker locker(&lock);

    if (!parent.isValid()) {
        cout << "[AgentModel::rowCount] vendor rows: " << vendors.size() << endl;
        return (int) vendors.size();
    }

    quint32 id(parent.internalId());
    IndexMap::const_iterator iter(linkage.find(id));
    if (iter == linkage.end())
        return 0;

    const AgentIndexPtr ai(iter->second);

    switch (ai->nodeType) {
    case NODE_VENDOR:
        cout << "[AgentModel::rowCount] Product rows:" << ai->children.size() << " id:" << ai->id << endl;
        return (int) ai->children.size();

    case NODE_PRODUCT:
        cout << "[AgentModel::rowCount] Instance rows:" << ai->agents.size() << " id:" << ai->id << endl;
        return (int) ai->agents.size();
    }

    return 0;
}


int AgentModel::columnCount(const QModelIndex &parent) const
{
    cout << "[AgentModel::columnCount]" << endl;
    return 1;
}


QVariant AgentModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (!index.isValid())
        return QVariant();

    quint32 id(index.internalId());
    IndexMap::const_iterator iter(linkage.find(id));
    if (iter == linkage.end()) {
        cout << "[AgentModel::data] Index with invalid ID: " << id << endl;
        return QVariant();
    }

    const AgentIndexPtr ai(iter->second);
    switch (ai->nodeType) {
    case NODE_VENDOR:
        cout << "[AgentModel::data] Returning vendor name '" << ai->text << "' for id:" << ai->id << " row:" << index.row() << endl;
        return QString(ai->text.c_str());
        break;

    case NODE_PRODUCT:
        break;
    }
}


QVariant AgentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
    case 0: return QString("Vendor");
    case 1: return QString("Product");
    case 2: return QString("Instance");
    case 3: return QString("Epoch");
    }

    return QVariant();
}


QModelIndex AgentModel::parent(const QModelIndex& index) const
{
    cout << "[AgentModel::parent] valid:" << (char) (index.isValid() ? 'Y' : 'N') << " id:" << index.internalId() << " row:" << index.row() << " col:" << index.column() << endl;
    return QModelIndex();
}


QModelIndex AgentModel::index(int row, int column, const QModelIndex &parent) const
{
    cout << "[AgentModel::index] row:" << row << " col:" << column << " valid-parent:" <<
        (char) (parent.isValid() ? 'Y' : 'N') << endl;
    if (!parent.isValid() && row <= vendors.size()) {
        //
        // Return the index of the vendor indicated by the row number.
        //
        int count(0);
        IndexList::const_iterator iter(vendors.begin());
        while (count < row) {
            iter++;
            count++;
        }

        cout << "[AgentModel::index]     id:" << (*iter)->id << endl;

        return createIndex(row, column, (*iter)->id);
    }

    return QModelIndex();
}

