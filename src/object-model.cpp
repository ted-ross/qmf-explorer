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

#include "object-model.h"
#include <qmf/SchemaId.h>
#include <qmf/DataAddr.h>
#include <iostream>

using std::cout;
using std::endl;

ObjectModel::ObjectModel(QObject* parent) : QAbstractItemModel(parent), nextId(1)
{
    // Intentionally Left Blank
}


void ObjectModel::renumber(IndexList& list)
{
    int sequence = 0;
    for (IndexList::iterator iter = list.begin(); iter != list.end(); iter++)
        (*iter)->row = sequence++;
}


ObjectModel::ObjectIndexPtr
ObjectModel::findOrInsertNode(IndexList& list, NodeType nodeType, ObjectIndexPtr parent,
                              const std::string& text, const qmf::Data& object, QModelIndex parentIndex,
                              IndexList::iterator& listPosition)
{
    ObjectIndexPtr node;
    int rowCount;

    IndexList::iterator iter(list.begin());
    rowCount = 0;
    while (iter != list.end() && text > (*iter)->text) {
        iter++;
        rowCount++;
    }

    if (iter == list.end() || text != (*iter)->text) {
        //
        // A new data record needs to be inserted in-order in the list.
        //
        beginInsertRows(parentIndex, rowCount, rowCount);
        node.reset(new ObjectIndex());
        node->id = nextId++;
        node->nodeType = nodeType;
        node->text = text;
        node->parent = parent;
        node->object = object;
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
        if ((*iter)->text == text) {
            listPosition = iter;
            break;
        }

    return node;
}


void ObjectModel::addObject(const qmf::Data& object)
{
    const qmf::SchemaId& schemaId(object.getSchemaId());
    const qmf::DataAddr& addr(object.getAddr());

    const std::string& package(schemaId.getPackageName());
    const std::string& schema(schemaId.getName());
    const std::string& instance(addr.getAgentName() + ":" + addr.getName());

    IndexList::iterator unused;

    ObjectIndexPtr pptr(findOrInsertNode(packages, NODE_PACKAGE, ObjectIndexPtr(),
                                         package, object, QModelIndex(), unused));
    ObjectIndexPtr sptr(findOrInsertNode(pptr->children, NODE_SCHEMA, pptr,
                                         schema, object, createIndex(pptr->row, 0, pptr->id), unused));
    ObjectIndexPtr iptr(findOrInsertNode(sptr->children, NODE_INSTANCE, sptr,
                                         instance, object, createIndex(sptr->row, 0, sptr->id), unused));
}


void ObjectModel::delObject(const qmf::Data& object)
{
    const qmf::SchemaId& schemaId(object.getSchemaId());
    const qmf::DataAddr& addr(object.getAddr());

    const std::string& package(schemaId.getPackageName());
    const std::string& schema(schemaId.getName());
    const std::string& instance(addr.getAgentName() + ":" + addr.getName());

    IndexList::iterator piter;
    IndexList::iterator siter;
    IndexList::iterator iiter;

    ObjectIndexPtr pptr(findOrInsertNode(packages, NODE_PACKAGE, ObjectIndexPtr(), package, object, QModelIndex(), piter));

    QModelIndex pindex(createIndex(pptr->row, 0, pptr->id));
    ObjectIndexPtr sptr(findOrInsertNode(pptr->children, NODE_SCHEMA, pptr, schema, object, pindex, siter));

    QModelIndex iindex(createIndex(sptr->row, 0, sptr->id));
    ObjectIndexPtr iptr(findOrInsertNode(sptr->children, NODE_INSTANCE, sptr, instance, object, iindex, iiter));

    beginRemoveRows(iindex, iptr->row, iptr->row);
    sptr->children.erase(iiter);
    linkage.erase(linkage.find(iptr->id));
    renumber(sptr->children);
    endRemoveRows();

    if (sptr->children.size() == 0) {
        beginRemoveRows(pindex, sptr->row, sptr->row);
        pptr->children.erase(siter);
        linkage.erase(linkage.find(sptr->id));
        renumber(pptr->children);
        endRemoveRows();

        if (pptr->children.size() == 0) {
            beginRemoveRows(QModelIndex(), pptr->row, pptr->row);
            packages.erase(piter);
            linkage.erase(linkage.find(pptr->id));
            renumber(packages);
            endRemoveRows();
        }
    }
}


void ObjectModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, packages.size() - 1);
    packages.clear();
    linkage.clear();
    endRemoveRows();
}


void ObjectModel::selected(const QModelIndex& index)
{
    //
    // Get the data record linked to the ID.
    //
    quint32 id(index.internalId());
    IndexMap::const_iterator iter(linkage.find(id));
    if (iter == linkage.end())
        return;
    const ObjectIndexPtr ptr(iter->second);

    //
    // The selected tree row is a valid instance.  Relay it outbound.
    //
    if (ptr->nodeType == NODE_INSTANCE)
        emit instSelected(ptr->object);
}


int ObjectModel::rowCount(const QModelIndex &parent) const
{
    //
    // If the parent is invalid (top-level), return the number of packages.
    //
    if (!parent.isValid())
        return (int) packages.size();

    //
    // Get the data record linked to the ID.
    //
    quint32 id(parent.internalId());
    IndexMap::const_iterator iter(linkage.find(id));
    if (iter == linkage.end())
        return 0;
    const ObjectIndexPtr ptr(iter->second);

    //
    // For parents that are package or schema, return the number of children.
    //
    switch (ptr->nodeType) {
    case NODE_PACKAGE:
    case NODE_SCHEMA:
        return (int) ptr->children.size();
    }

    //
    // For instance nodes, return 0 because there are no children.
    //
    return 0;
}


int ObjectModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}


QVariant ObjectModel::data(const QModelIndex &index, int role) const
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
    const ObjectIndexPtr ptr(liter->second);
    return QString(ptr->text.c_str());
}


QVariant ObjectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0 && role == Qt::DisplayRole && orientation == Qt::Horizontal)
        return QString("Objects");
    return QVariant();
}


QModelIndex ObjectModel::parent(const QModelIndex& index) const
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
    ObjectIndexPtr ptr(iter->second);

    //
    // Handle the package case
    //
    if (ptr->nodeType == NODE_PACKAGE)
        return QModelIndex();

    //
    // Handle the schema and instance level cases
    //
    return createIndex(ptr->parent->row, 0, ptr->parent->id);
}


QModelIndex ObjectModel::index(int row, int column, const QModelIndex &parent) const
{
    int count;
    IndexList::const_iterator iter;

    if (!parent.isValid()) {
        //
        // Handle the package-level case
        //
        count = 0;
        iter = packages.begin();
        while (iter != packages.end() && count < row) {
            count++;
            iter++;
        }

        if (iter == packages.end())
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
    ObjectIndexPtr ptr(link->second);

    //
    // Create an index for the child data record.
    //
    switch (ptr->nodeType) {
    case NODE_PACKAGE:
    case NODE_SCHEMA:
        count = 0;
        iter = ptr->children.begin();
        while (count < row) {
            iter++;
            count++;
        }

        if (iter == packages.end())
            return QModelIndex();
        return createIndex(row, 0, (*iter)->id);
    }

    return QModelIndex();
}

