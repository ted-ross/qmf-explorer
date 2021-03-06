#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

PROJECT(qmfe)
FIND_PACKAGE(Qt4 REQUIRED)

SET(qmfe_FORMS explorer_main.ui)

SET(qmfe_HEADERS
    main.h
    qmf-thread.h
    agent-model.h
    agent-detail-model.h
    object-model.h
    object-detail-model.h
    )

SET(qmfe_SOURCES
    main.cpp
    qmf-thread.cpp
    agent-model.cpp
    agent-detail-model.cpp
    object-model.cpp
    object-detail-model.cpp
    )

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

QT4_WRAP_UI(qmfe_FORMS_HEADERS ${qmfe_FORMS})
QT4_WRAP_CPP(qmfe_HEADERS_MOC ${qmfe_HEADERS})

ADD_EXECUTABLE(qmfe ${qmfe_SOURCES} ${qmfe_HEADERS_MOC} ${qmfe_FORMS_HEADERS})
TARGET_LINK_LIBRARIES(qmfe ${QT_LIBRARIES} qmf2 qpidmessaging qpidtypes)

