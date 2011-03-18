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


void AgentModel::renumber(IndexList& list)
{
    int sequence = 0;
    for (IndexList::iterator iter = list.begin(); iter != list.end(); iter++)
        (*iter)->row = sequence++;
}


AgentModel::AgentIndexPtr
AgentModel::findOrInsertNode(IndexList& list, NodeType nodeType, AgentIndexPtr parent,
                             const std::string& text, const qmf::Agent& agent, QModelIndex parentIndex,
                             IndexList::iterator& listPosition)
{
    AgentIndexPtr node;
    int rowCount;
    std::string insertText(text.empty() ? agent.getInstance() : text);

    IndexList::iterator iter(list.begin());
    rowCount = 0;
    while (iter != list.end() && insertText > (*iter)->text) {
        iter++;
        rowCount++;
    }

    if (iter == list.end() || insertText != (*iter)->text) {
        //
        // A new data record needs to be inserted in-order in the list.
        //
        beginInsertRows(parentIndex, rowCount, rowCount);
        node.reset(new AgentIndex());
        node->id = nextId++;
        node->nodeType = nodeType;
        node->text = insertText;
        node->parent = parent;
        linkage[node->id] = node;

        if (iter == list.end())
            list.push_back(node);
        else
            list.insert(iter, node);
        renumber(list);
        endInsertRows();
    } else
        node = *iter;

    for (iter = list.begin(); iter != list.end(); iter++)
        if ((*iter)->text == insertText) {
            listPosition = iter;
            break;
        }

    return node;
}


void AgentModel::addAgent(const qmf::Agent& agent)
{
    const std::string& vendor(agent.getVendor());
    const std::string& product(agent.getProduct());
    const std::string& instance(agent.getInstance());
    IndexList::iterator unused;

    AgentIndexPtr vptr(findOrInsertNode(vendors, NODE_VENDOR, AgentIndexPtr(),
                                        vendor, agent, QModelIndex(), unused));
    AgentIndexPtr pptr(findOrInsertNode(vptr->children, NODE_PRODUCT, vptr,
                                        product, agent, createIndex(vptr->row, 0, vptr->id), unused));
    AgentIndexPtr iptr(findOrInsertNode(pptr->children, NODE_INSTANCE, pptr,
                                        instance, agent, createIndex(pptr->row, 0, pptr->id), unused));
}


void AgentModel::delAgent(const qmf::Agent& agent)
{
    const std::string& vendor(agent.getVendor());
    const std::string& product(agent.getProduct());
    const std::string& instance(agent.getInstance());
    IndexList::iterator viter;
    IndexList::iterator piter;
    IndexList::iterator iiter;

    AgentIndexPtr vptr(findOrInsertNode(vendors, NODE_VENDOR, AgentIndexPtr(), vendor, agent, QModelIndex(), viter));

    QModelIndex pindex(createIndex(vptr->row, 0, vptr->id));
    AgentIndexPtr pptr(findOrInsertNode(vptr->children, NODE_PRODUCT, vptr, product, agent, pindex, piter));

    QModelIndex iindex(createIndex(pptr->row, 0, pptr->id));
    AgentIndexPtr iptr(findOrInsertNode(pptr->children, NODE_INSTANCE, pptr, instance, agent, iindex, iiter));

    beginRemoveRows(iindex, iptr->row, iptr->row);
    pptr->children.erase(iiter);
    linkage.erase(linkage.find(iptr->id));
    renumber(pptr->children);
    endRemoveRows();

    if (pptr->children.size() == 0) {
        beginRemoveRows(pindex, pptr->id, pptr->id);
        vptr->children.erase(piter);
        linkage.erase(linkage.find(pptr->id));
        renumber(vptr->children);
        endRemoveRows();

        if (vptr->children.size() == 0) {
            beginRemoveRows(QModelIndex(), vptr->row, vptr->row);
            vendors.erase(viter);
            linkage.erase(linkage.find(vptr->id));
            renumber(vendors);
            endRemoveRows();
        }
    }
}


void AgentModel::clear()
{
    vendors.clear();
    linkage.clear();
}


int AgentModel::rowCount(const QModelIndex &parent) const
{
    //
    // If the parent is invalid (top-level), return the number of vendors.
    //
    if (!parent.isValid())
        return (int) vendors.size();

    //
    // Get the data record linked to the ID.
    //
    quint32 id(parent.internalId());
    IndexMap::const_iterator iter(linkage.find(id));
    if (iter == linkage.end())
        return 0;
    const AgentIndexPtr ptr(iter->second);

    //
    // For parents that are vendor or product, return the number of children.
    //
    switch (ptr->nodeType) {
    case NODE_VENDOR:
    case NODE_PRODUCT:
        return (int) ptr->children.size();
    }

    //
    // For instance nodes, return 0 because there are no children.
    //
    return 0;
}


int AgentModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}


QVariant AgentModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (!index.isValid())
        return QVariant();

    //
    // Note that we don't look at the row number in this function.  The index structure
    // is defined such that the internalId is sufficient to identify the data record being
    // interrogated.
    //

    //
    // Get the data record linked to the ID.
    //
    quint32 id(index.internalId());
    IndexMap::const_iterator liter(linkage.find(id));
    if (liter == linkage.end())
        return QVariant();
    const AgentIndexPtr ptr(liter->second);
    return QString(ptr->text.c_str());
}


QVariant AgentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}


QModelIndex AgentModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    quint32 id(index.internalId());

    //
    // Get the linked record
    //
    IndexMap::const_iterator iter(linkage.find(id));
    if (iter == linkage.end())
        return QModelIndex();
    AgentIndexPtr ptr(iter->second);

    //
    // Handle the vendor case
    //
    if (ptr->nodeType == NODE_VENDOR)
        return QModelIndex();

    //
    // Handle the product and instance level cases
    //
    return createIndex(ptr->parent->row, 0, ptr->parent->id);
}


QModelIndex AgentModel::index(int row, int column, const QModelIndex &parent) const
{
    int count;
    IndexList::const_iterator iter;

    if (!parent.isValid()) {
        //
        // Handle the vendor-level case
        //
        count = 0;
        iter = vendors.begin();
        while (iter != vendors.end() && count < row) {
            count++;
            iter++;
        }

        if (iter == vendors.end())
            return QModelIndex();
        return createIndex(row, 0, (*iter)->id);
    }

    //
    // Get the data record linked to the ID.
    //
    quint32 id(parent.internalId());
    IndexMap::const_iterator link = linkage.find(id);
    if (link == linkage.end())
        return QModelIndex();
    AgentIndexPtr ptr(link->second);

    //
    // Create an index for the child data record.
    //
    switch (ptr->nodeType) {
    case NODE_VENDOR:
    case NODE_PRODUCT:
        count = 0;
        iter = ptr->children.begin();
        while (count < row) {
            iter++;
            count++;
        }

        if (iter == vendors.end())
            return QModelIndex();
        return createIndex(row, 0, (*iter)->id);
    }

    return QModelIndex();
}

