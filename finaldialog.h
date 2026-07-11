#ifndef FINALDIALOG_H
#define FINALDIALOG_H

#include <QDialog>

namespace Ui {
class FinalDialog;
}

class FinalDialog : public QDialog
{
    Q_OBJECT

public:
    FinalDialog(QWidget *parent = 0);
    ~FinalDialog();

    void setResult(const QString &text);

signals:
    void backToMenu();

private slots:
    void onBackToMenuClicked();

private:
    Ui::FinalDialog *ui;
};

#endif // FINALDIALOG_H