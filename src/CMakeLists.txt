
include(FindQt4)
include(UseQt4)

if(QT4_FOUND)
   include_directories(${QT_QTGUI_INCLUDE_DIR})
else(pcre_FOUND)
   message(FATAL_ERROR "QT4 header/library not found.")
endif(QT4_FOUND)

set(QMFE_SOURCES
    main.cpp
    )

add_executable(qmfe ${QMFE_SOURCES})
target_link_libraries(qmfe ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} qmf2 qpidmessaging)

