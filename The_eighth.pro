QMAKE_RESOURCE_FLAGS += -no-compress
CONFIG += resources_big

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = The_eighth
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    gamecontroller.cpp \
    mazewidget.cpp \
    linkgamewidget.cpp \
    mariowidget.cpp \
    hollowknightwidget.cpp \
    leaderboarddialog.cpp \
    inputnamedialog.cpp \
    backgrounddialog.cpp \
    gameresultdialog.cpp \
    gameintrodialog.cpp \
    levelstartdialog.cpp \
    level1startdialog.cpp \
    level2startdialog.cpp \
    level3startdialog.cpp \
    game1introdialog.cpp \
    game2introdialog.cpp \
    game3introdialog.cpp \
    result1_1dialog.cpp \
    result1_2dialog.cpp \
    result1_3dialog.cpp \
    result2_1dialog.cpp \
    result2_2dialog.cpp \
    result2_3dialog.cpp \
    result3_1dialog.cpp \
    result3_2dialog.cpp \
    result3_3dialog.cpp \
    finaldialog.cpp

HEADERS += \
    config.h \
    mainwindow.h \
    gamecontroller.h \
    mazewidget.h \
    linkgamewidget.h \
    mariowidget.h \
    hollowknightwidget.h \
    leaderboarddialog.h \
    inputnamedialog.h \
    backgrounddialog.h \
    gameresultdialog.h \
    gameintrodialog.h \
    levelstartdialog.h \
    level1startdialog.h \
    level2startdialog.h \
    level3startdialog.h \
    game1introdialog.h \
    game2introdialog.h \
    game3introdialog.h \
    result1_1dialog.h \
    result1_2dialog.h \
    result1_3dialog.h \
    result2_1dialog.h \
    result2_2dialog.h \
    result2_3dialog.h \
    result3_1dialog.h \
    result3_2dialog.h \
    result3_3dialog.h \
    finaldialog.h

RESOURCES += \
    resources.qrc

DESTDIR = $$PWD/release
OBJECTS_DIR = $$PWD/release/obj
MOC_DIR = $$PWD/release/moc
RCC_DIR = $$PWD/release/rcc
UI_DIR = $$PWD/release/ui

QMAKE_CXXFLAGS += -std=c++11

FORMS += \
    mainwindow.ui \
    inputnamedialog.ui \
    backgrounddialog.ui \
    leaderboarddialog.ui \
    gameresultdialog.ui \
    gameintrodialog.ui \
    levelstartdialog.ui \
    level1startdialog.ui \
    level2startdialog.ui \
    level3startdialog.ui \
    game1introdialog.ui \
    game2introdialog.ui \
    game3introdialog.ui \
    result1_1dialog.ui \
    result1_2dialog.ui \
    result1_3dialog.ui \
    result2_1dialog.ui \
    result2_2dialog.ui \
    result2_3dialog.ui \
    result3_1dialog.ui \
    result3_2dialog.ui \
    result3_3dialog.ui \
    finaldialog.ui
