#ifndef _qe_object_model_h
#define _qe_object_model_h
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
#include <QStringList>
#include <qmf/Data.h>
#include <sstream>
#include <string>
#include <map>
#include <list>
#include <deque>
#include <boost/shared_ptr.hpp>

Q_DECLARE_METATYPE(qmf::Data);

class ObjectModel : public QAbstractItemModel {
    Q_OBJECT

public:
    ObjectModel(QObject* parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex parent(const QModelIndex& index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

public slots:
    void addPackage(const QString&);
    void addClass(const QStringList&);
    void addObject(const qmf::Data&);
    void delObject(const qmf::Data&);
    void clear();
    void selected(const QModelIndex&);

signals:
    void instSelected(const qmf::Data&);

private:
    typedef enum { NODE_PACKAGE, NODE_SCHEMA, NODE_INSTANCE } NodeType;
    struct ObjectIndex;
    typedef boost::shared_ptr<ObjectIndex> ObjectIndexPtr;
    typedef std::map<quint32, ObjectIndexPtr> IndexMap;
    typedef std::list<ObjectIndexPtr> IndexList;

    struct ObjectIndex {
        quint32 id;
        int row;
        NodeType nodeType;
        std::string text;
        ObjectIndexPtr parent;
        IndexList children;
        qmf::Data object;
    };

    IndexList packages;
    IndexMap linkage;
    quint32 nextId;

    void renumber(IndexList&);
    ObjectIndexPtr findOrInsertNode(IndexList&, NodeType, ObjectIndexPtr, const std::string&,
                                   const qmf::Data&, QModelIndex, IndexList::iterator&);
};

#endif

