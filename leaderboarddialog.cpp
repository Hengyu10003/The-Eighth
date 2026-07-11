#include "leaderboarddialog.h"
#include "ui_leaderboarddialog.h"
#include <QFile>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <algorithm>

LeaderboardDialog::LeaderboardDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LeaderboardDialog)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "排名" << "昵称" << "碎片数" << "完成时间");
    ui->tableWidget->setColumnWidth(0, 60);
    ui->tableWidget->setColumnWidth(1, 150);
    ui->tableWidget->setColumnWidth(2, 120);
    ui->tableWidget->setColumnWidth(3, 130);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->backBtn, SIGNAL(clicked()), this, SLOT(onBackClicked()));

    loadLeaderboard();
}

LeaderboardDialog::~LeaderboardDialog()
{
    delete ui;
}

void LeaderboardDialog::loadLeaderboard()
{
    QFile file("leaderboard.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(",");
            if (parts.size() == 3) {
                Score score;
                score.name = parts[0];
                score.fragments = parts[1].toInt();
                score.time = parts[2].toInt();
                scores.append(score);
            }
        }
        file.close();
    }

    std::sort(scores.begin(), scores.end(), [](const Score &a, const Score &b) {
        if (a.fragments != b.fragments) {
            return a.fragments > b.fragments;
        }
        return a.time < b.time;
    });

    ui->tableWidget->setRowCount(scores.size());
    for (int i = 0; i < scores.size(); i++) {
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(scores[i].name));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(scores[i].fragments)));
        ui->tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(scores[i].time) + "秒"));
    }
}

void LeaderboardDialog::saveLeaderboard()
{
    QFile file("leaderboard.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const Score &score : scores) {
            out << score.name << "," << score.fragments << "," << score.time << "\n";
        }
        file.close();
    }
}

void LeaderboardDialog::addScore(const QString &name, int fragments, int time)
{
    bool found = false;
    for (Score &score : scores) {
        if (score.name == name) {
            if (fragments > score.fragments || (fragments == score.fragments && time < score.time)) {
                score.fragments = fragments;
                score.time = time;
            }
            found = true;
            break;
        }
    }

    if (!found) {
        Score score;
        score.name = name;
        score.fragments = fragments;
        score.time = time;
        scores.append(score);
    }

    std::sort(scores.begin(), scores.end(), [](const Score &a, const Score &b) {
        if (a.fragments != b.fragments) {
            return a.fragments > b.fragments;
        }
        return a.time < b.time;
    });

    saveLeaderboard();
    loadLeaderboard();
}

void LeaderboardDialog::onBackClicked()
{
    reject();
}
