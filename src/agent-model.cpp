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
        if (addAgentQueue.empty())
            return;
        agent = addAgentQueue.front();
        addAgentQueue.pop_front();
    }

    int rowCount;
    const std::string& vendor(agent.getVendor());
    const std::string& product(agent.getProduct());
    const std::string& instance(agent.getInstance());

    AgentIndexPtr vptr;
    AgentIndexPtr pptr;

    IndexList::iterator iter(vendors.begin());
    rowCount = 0;
    while (iter != vendors.end() && vendor > (*iter)->text) {
        iter++;
        rowCount++;
    }

    if (iter == vendors.end() || vendor != (*iter)->text) {
        //
        // We need to insert a new vendor row into the model
        //
        beginInsertRows(QModelIndex(), rowCount, rowCount);

        //
        // Allocate a new AgentIndex
        //
        vptr.reset(new AgentIndex());
        vptr->id = nextId++;
        vptr->row = rowCount;
        vptr->column = 0;
        vptr->nodeType = NODE_VENDOR;
        vptr->text = vendor;
        linkage[vptr->id] = vptr;

        if (iter == vendors.end())
            vendors.push_back(vptr);
        else
            vendors.insert(iter, vptr);
        endInsertRows();
    } else
        vptr = *iter;

    iter = vptr->children.begin();
    rowCount = 0;
    while (iter != vptr->children.end() && product > (*iter)->text) {
        iter++;
        rowCount++;
    }

    if (iter == vptr->children.end() || product != (*iter)->text) {
        //
        // We need to insert a new product into the vendor
        //
        beginInsertRows(createIndex(vptr->row, vptr->column, vptr->id), rowCount, rowCount);

        //
        // Allocate a new AgentIndex
        //
        pptr.reset(new AgentIndex());
        pptr->id = nextId++;
        pptr->row = rowCount;
        pptr->column = 0;
        pptr->nodeType = NODE_PRODUCT;
        pptr->text = product;
        pptr->parent = vptr;
        linkage[pptr->id] = pptr;

        if (iter == vptr->children.end())
            vptr->children.push_back(pptr);
        else
            vptr->children.insert(iter, pptr);
        endInsertRows();
    } else
        pptr = *iter;
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
    case NODE_PRODUCT:
        return QString(ai->text.c_str());
        break;
    }
}


QVariant AgentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
#if 0
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
#endif
    return QVariant();
}


QModelIndex AgentModel::parent(const QModelIndex& index) const
{
    cout << "[AgentModel::parent] valid:" << (char) (index.isValid() ? 'Y' : 'N') << " id:" << index.internalId() << " row:" << index.row() << " col:" << index.column() << endl;

    if (!index.isValid())
        return QModelIndex();

    quint32 id(index.internalId());
    IndexMap::const_iterator iter(linkage.find(id));
    if (iter == linkage.end())
        return QModelIndex();

    AgentIndexPtr ptr(iter->second);
    if (ptr->nodeType == NODE_PRODUCT)
        return createIndex(ptr->row, ptr->column, ptr->parent->id);

    return QModelIndex();
}


QModelIndex AgentModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid() && row < vendors.size()) {
        //
        // Return the index of the vendor indicated by the row number.
        //
        int count(0);
        IndexList::const_iterator iter(vendors.begin());
        while (count < row) {
            iter++;
            count++;
        }

        return createIndex(row, column, (*iter)->id);
    }

    quint32 id(parent.internalId());
    IndexMap::const_iterator iter(linkage.find(id));
    if (iter == linkage.end())
        return QModelIndex();

    AgentIndexPtr ptr(iter->second);

    if (ptr->nodeType == NODE_VENDOR) {
        if (row >= ptr->children.size())
            return QModelIndex();

        int count(0);
        IndexList::const_iterator iter(ptr->children.begin());
        while (count < row) {
            iter++;
            count++;
        }

        (*iter)->row = row;
        (*iter)->column = column;
        return createIndex(row, column, (*iter)->id);
    }

    return QModelIndex();
}

