#include "gamecontroller.h"
#include "mainwindow.h"
#include "level1startdialog.h"
#include "level2startdialog.h"
#include "level3startdialog.h"
#include "result1_1dialog.h"
#include "result1_2dialog.h"
#include "result1_3dialog.h"
#include "result2_1dialog.h"
#include "result2_2dialog.h"
#include "result2_3dialog.h"
#include "result3_1dialog.h"
#include "result3_2dialog.h"
#include "result3_3dialog.h"
#include "leaderboarddialog.h"

#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

GameController::GameController(MainWindow *mainWindow)
    : QObject(mainWindow)
    , mainWindow(mainWindow)
    , level1StartDialog(nullptr)
    , level2StartDialog(nullptr)
    , level3StartDialog(nullptr)
{
}

GameController::~GameController()
{
    delete level1StartDialog;
    delete level2StartDialog;
    delete level3StartDialog;
}

void GameController::onMazeCompleted()
{
    QTimer::singleShot(0, this, [this]() {
        showLevelStart(mainWindow->getCurrentLevel());
    });
}

void GameController::onLinkGameWon()
{
    mainWindow->addFragment();
    handleGameResult(true, 1, mainWindow->getCurrentDifficulty());
}

void GameController::onLinkGameLost()
{
    handleGameResult(false, 1, mainWindow->getCurrentDifficulty());
}

void GameController::onMarioGameWon()
{
    mainWindow->addFragment();
    handleGameResult(true, 2, mainWindow->getCurrentDifficulty());
}

void GameController::onMarioGameLost()
{
    handleGameResult(false, 2, mainWindow->getCurrentDifficulty());
}

void GameController::onHollowKnightWon()
{
    mainWindow->addFragment();
    handleGameResult(true, 3, mainWindow->getCurrentDifficulty());
}

void GameController::onHollowKnightLost()
{
    handleGameResult(false, 3, mainWindow->getCurrentDifficulty());
}

void GameController::onLevelStart(int gameType)
{
    Q_UNUSED(gameType);
    int level = mainWindow->getCurrentLevel();
    int difficulty = mainWindow->getCurrentDifficulty();

    switch (level) {
    case 1:
        mainWindow->showLinkGame(difficulty);
        break;
    case 2:
        mainWindow->showMarioGame(difficulty);
        break;
    case 3:
        mainWindow->showHollowKnightGame(difficulty);
        break;
    }
}

void GameController::onGameResult(int result)
{
    int level = mainWindow->getCurrentLevel();
    int difficulty = mainWindow->getCurrentDifficulty();

    if (result == 0) {
        if (difficulty < 3) {
            mainWindow->setCurrentDifficulty(difficulty + 1);
            showLevelStart(level);
        } else {
            if (level < 3) {
                mainWindow->setCurrentLevel(level + 1);
                mainWindow->setCurrentDifficulty(1);
                mainWindow->showMaze(mainWindow->getCurrentLevel());
            } else {
                returnToMainMenu();
            }
        }

    } else if (result == 1) {
        int curLevel = mainWindow->getCurrentLevel();
        int curDifficulty = mainWindow->getCurrentDifficulty();
        if (curLevel == 1)
            mainWindow->showLinkGame(curDifficulty);
        else if (curLevel == 2)
            mainWindow->showMarioGame(curDifficulty);
        else if (curLevel == 3)
            mainWindow->showHollowKnightGame(curDifficulty);

    } else if (result == 2) {
        int curLevel = mainWindow->getCurrentLevel();
        if (curLevel >= 2) {
            showLevelStart(curLevel);
        } else {
            returnToMainMenu();
        }

    } else if (result == 3) {
        int curLevel = mainWindow->getCurrentLevel();
        if (curLevel < 3) {
            mainWindow->setCurrentLevel(curLevel + 1);
            mainWindow->setCurrentDifficulty(1);
            mainWindow->showMaze(mainWindow->getCurrentLevel());
        } else {
            QMessageBox msgBox(mainWindow);
            msgBox.setWindowTitle(QString::fromUtf8("\xe6\x81\xad\xe5\x96\x9c"));
            msgBox.setText(QString::fromUtf8(
                "\xe6\x81\xad\xe5\x96\x9c\xe9\x80\x9a\xe5\x85\xb3\xef\xbc\x81\n"
                "\xe4\xbd\xa0\xe5\xb7\xb2\xe5\xae\x8c\xe6\x88\x90\xe6\x89\x80\xe6\x9c\x89\xe5\x85\xb3\xe5\x8d\xa1\xef\xbc\x81"
            ));
            msgBox.setIcon(QMessageBox::Information);
            msgBox.addButton(QString::fromUtf8("\xe7\xa1\xae\xe5\xae\x9a"), QMessageBox::AcceptRole);
            msgBox.exec();
            returnToMainMenu();
        }
    }
}

void GameController::showLevelStart(int level)
{
    switch (level) {
    case 1:
        delete level1StartDialog;
        level1StartDialog = new Level1StartDialog(mainWindow);
        connect(level1StartDialog, SIGNAL(accepted()), this, SLOT(onLevel1Start()));
        connect(level1StartDialog, SIGNAL(rejected()), this, SLOT(returnToMainMenu()));
        level1StartDialog->exec();
        break;
    case 2:
        delete level2StartDialog;
        level2StartDialog = new Level2StartDialog(mainWindow);
        connect(level2StartDialog, SIGNAL(accepted()), this, SLOT(onLevel2Start()));
        connect(level2StartDialog, SIGNAL(rejected()), this, SLOT(returnToMainMenu()));
        level2StartDialog->exec();
        break;
    case 3:
        delete level3StartDialog;
        level3StartDialog = new Level3StartDialog(mainWindow);
        connect(level3StartDialog, SIGNAL(accepted()), this, SLOT(onLevel3Start()));
        connect(level3StartDialog, SIGNAL(rejected()), this, SLOT(returnToMainMenu()));
        level3StartDialog->exec();
        break;
    }
}

void GameController::onLevel1Start()
{
    mainWindow->showLinkGame(mainWindow->getCurrentDifficulty());
}

void GameController::onLevel2Start()
{
    mainWindow->showMarioGame(mainWindow->getCurrentDifficulty());
}

void GameController::onLevel3Start()
{
    mainWindow->showHollowKnightGame(mainWindow->getCurrentDifficulty());
}

void GameController::returnToMainMenu()
{
    mainWindow->stopGameTimer();
    saveLeaderboardEntry(mainWindow->getPlayerName(),
                         mainWindow->getCollectedFragments(),
                         mainWindow->getGameTime());
    QTimer::singleShot(0, mainWindow, [this]() {
        mainWindow->showMainMenu();
    });
}

void GameController::handleGameResult(bool won, int level, int difficulty)
{
    QDialog *dlg = nullptr;

    switch (level) {
    case 1:
        switch (difficulty) {
        case 1: {
            Result1_1Dialog *d = new Result1_1Dialog(mainWindow);
            d->setResult(won);
            connect(d, SIGNAL(resultSelected(int)), this, SLOT(onGameResult(int)));
            dlg = d;
            break;
        }
        case 2: {
            Result1_2Dialog *d = new Result1_2Dialog(mainWindow);
            d->setResult(won);
            connect(d, SIGNAL(resultSelected(int)), this, SLOT(onGameResult(int)));
            dlg = d;
            break;
        }
        case 3: {
            Result1_3Dialog *d = new Result1_3Dialog(mainWindow);
            d->setResult(won);
            connect(d, SIGNAL(resultSelected(int)), this, SLOT(onGameResult(int)));
            dlg = d;
            break;
        }
        }
        break;
    case 2:
        switch (difficulty) {
        case 1: {
            Result2_1Dialog *d = new Result2_1Dialog(mainWindow);
            d->setResult(won);
            connect(d, SIGNAL(resultSelected(int)), this, SLOT(onGameResult(int)));
            dlg = d;
            break;
        }
        case 2: {
            Result2_2Dialog *d = new Result2_2Dialog(mainWindow);
            d->setResult(won);
            connect(d, SIGNAL(resultSelected(int)), this, SLOT(onGameResult(int)));
            dlg = d;
            break;
        }
        case 3: {
            Result2_3Dialog *d = new Result2_3Dialog(mainWindow);
            d->setResult(won);
            connect(d, SIGNAL(resultSelected(int)), this, SLOT(onGameResult(int)));
            dlg = d;
            break;
        }
        }
        break;
    case 3:
        switch (difficulty) {
        case 1: {
            Result3_1Dialog *d = new Result3_1Dialog(mainWindow);
            d->setResult(won);
            connect(d, SIGNAL(resultSelected(int)), this, SLOT(onGameResult(int)));
            dlg = d;
            break;
        }
        case 2: {
            Result3_2Dialog *d = new Result3_2Dialog(mainWindow);
            d->setResult(won);
            connect(d, SIGNAL(resultSelected(int)), this, SLOT(onGameResult(int)));
            dlg = d;
            break;
        }
        case 3: {
            Result3_3Dialog *d = new Result3_3Dialog(mainWindow);
            d->setResult(won);
            connect(d, SIGNAL(resultSelected(int)), this, SLOT(onGameResult(int)));
            dlg = d;
            break;
        }
        }
        break;
    }

    if (dlg) {
        dlg->exec();
        delete dlg;
    }
}

void GameController::saveLeaderboardEntry(const QString &name, int fragments, int time)
{
    struct ScoreEntry { QString name; int fragments; int time; };
    QList<ScoreEntry> scores;

    QFile file("leaderboard.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(",");
            if (parts.size() == 3) {
                ScoreEntry s;
                s.name = parts[0];
                s.fragments = parts[1].toInt();
                s.time = parts[2].toInt();
                scores.append(s);
            }
        }
        file.close();
    }

    bool found = false;
    for (ScoreEntry &s : scores) {
        if (s.name == name) {
            if (fragments > s.fragments || (fragments == s.fragments && time < s.time)) {
                s.fragments = fragments;
                s.time = time;
            }
            found = true;
            break;
        }
    }
    if (!found) {
        scores.append({name, fragments, time});
    }

    std::sort(scores.begin(), scores.end(), [](const ScoreEntry &a, const ScoreEntry &b) {
        if (a.fragments != b.fragments) return a.fragments > b.fragments;
        return a.time < b.time;
    });

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const ScoreEntry &s : scores) {
            out << s.name << "," << s.fragments << "," << s.time << "\n";
        }
        file.close();
    }
}
