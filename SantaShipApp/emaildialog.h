#ifndef EMAILDIALOG_H
#define EMAILDIALOG_H

#include <QModelIndexList>
#include <QDialog>

namespace Ui {
class EmailDialog;
}

class EmailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmailDialog(QWidget *parent = 0);
    ~EmailDialog();

    void setEmailAddress(QString emailAddress);
    QString emailAddress();

    void setFileNames(QStringList indexList);
    QStringList fileNames();

public slots:
    int exec();

private:
    Ui::EmailDialog *ui;
    QStringList _fileNames;
};

#endif // EMAILDIALOG_H
