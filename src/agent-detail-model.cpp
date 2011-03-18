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

#include "agent-detail-model.h"
#include <iostream>

using std::cout;
using std::endl;

AgentDetailModel::AgentDetailModel(QObject* parent) : QAbstractItemModel(parent)
{
    // Intentionally Left Blank
}


void AgentDetailModel::newAgent(const qmf::Agent& agent)
{
    if (!agent.isValid())
        return;

    clear();

    const qpid::types::Variant::Map& attrs(agent.getAttributes());

    beginInsertRows(QModelIndex(), 0, attrs.size() - 1);
    for (qpid::types::Variant::Map::const_iterator iter = attrs.begin();
         iter != attrs.end(); iter++) {
        keys << QString(iter->first.c_str());
        values << QString(iter->second.asString().c_str());
    }
    endInsertRows();
}


void AgentDetailModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, keys.size() - 1);
    keys.clear();
    values.clear();
    endRemoveRows();
}


int AgentDetailModel::rowCount(const QModelIndex &parent) const
{
    //
    // If the parent is invalid (top-level), return the number of attributes.
    //
    if (!parent.isValid())
        return (int) keys.size();

    //
    // This is not a tree so there are not child rows.
    //
    return 0;
}


int AgentDetailModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}


QVariant AgentDetailModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (!index.isValid())
        return QVariant();

    switch (index.column()) {
    case 0: return keys.at(index.row());
    case 1: return values.at(index.row());
    }
}


QVariant AgentDetailModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    switch (section) {
    case 0: return QString("Key");
    case 1: return QString("Value");
    }

    return QVariant();
}


QModelIndex AgentDetailModel::parent(const QModelIndex& index) const
{
    //
    // Not a tree structure, no parents.
    //
    return QModelIndex();
}


QModelIndex AgentDetailModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column);

    return QModelIndex();
}

