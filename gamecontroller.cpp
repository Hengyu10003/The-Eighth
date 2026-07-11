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
#include "gameresultdialog.h"
#include "finaldialog.h"
#include "leaderboarddialog.h"

#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

GameController::GameController(MainWindow *mainWindow)
    : QObject(mainWindow)
    , mainWindow(mainWindow)
    , gameResultDialog(nullptr)
{
}

GameController::~GameController()
{
    delete gameResultDialog;
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
        if (curLevel == 3) {
            FinalDialog dlg(mainWindow);
            dlg.setResult("恭喜通关所有关卡！");
            connect(&dlg, &FinalDialog::backToMenu, [this]() {
                mainWindow->stopGameTimer();
                saveLeaderboardEntry(mainWindow->getPlayerName(),
                                     mainWindow->getCollectedFragments(),
                                     mainWindow->getGameTime());
                mainWindow->showMainMenu();
            });
            dlg.exec();
        } else if (curLevel >= 2) {
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
            FinalDialog dlg(mainWindow);
            dlg.setResult("恭喜通关所有关卡！");
            connect(&dlg, &FinalDialog::backToMenu, [this]() {
                mainWindow->stopGameTimer();
                saveLeaderboardEntry(mainWindow->getPlayerName(),
                                     mainWindow->getCollectedFragments(),
                                     mainWindow->getGameTime());
                mainWindow->showMainMenu();
            });
            dlg.exec();
        }
    }
}

void GameController::showLevelStart(int level)
{
    // 根据关卡使用对应的 UI 设计对话框
    QDialog *dialog = nullptr;
    if (level == 1) {
        dialog = new Level1StartDialog(mainWindow);
    } else if (level == 2) {
        dialog = new Level2StartDialog(mainWindow);
    } else {
        dialog = new Level3StartDialog(mainWindow);
    }

    int result = dialog->exec();
    dialog->deleteLater();

    if (result == QDialog::Rejected) {
        returnToMainMenu();
    } else {
        onLevelStart(level);
    }
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
    if (won) {
        // 胜利：根据关卡和难度使用对应 UI 对话框
        QDialog *dialog = nullptr;
        if (level == 1 && difficulty == 1)
            dialog = new Result1_1Dialog(mainWindow);
        else if (level == 1 && difficulty == 2)
            dialog = new Result1_2Dialog(mainWindow);
        else if (level == 1 && difficulty == 3)
            dialog = new Result1_3Dialog(mainWindow);
        else if (level == 2 && difficulty == 1)
            dialog = new Result2_1Dialog(mainWindow);
        else if (level == 2 && difficulty == 2)
            dialog = new Result2_2Dialog(mainWindow);
        else if (level == 2 && difficulty == 3)
            dialog = new Result2_3Dialog(mainWindow);
        else if (level == 3 && difficulty == 1)
            dialog = new Result3_1Dialog(mainWindow);
        else if (level == 3 && difficulty == 2)
            dialog = new Result3_2Dialog(mainWindow);
        else if (level == 3 && difficulty == 3)
            dialog = new Result3_3Dialog(mainWindow);

        if (dialog) {
            connect(dialog, SIGNAL(resultSelected(int)), this, SLOT(onGameResult(int)));
            dialog->exec();
            dialog->deleteLater();
        }
    } else {
        // 失败：使用通用失败对话框（直接显示按钮，无需确认步骤）
        if (!gameResultDialog) {
            gameResultDialog = new GameResultDialog(mainWindow);
            connect(gameResultDialog, SIGNAL(resultSelected(int)), this, SLOT(onGameResult(int)));
        }
        gameResultDialog->setResult(won, level, difficulty);
        gameResultDialog->exec();
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
