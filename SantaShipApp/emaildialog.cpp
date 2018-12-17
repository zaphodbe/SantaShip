#include "emaildialog.h"
#include "ui_emaildialog.h"

#include <QDebug>
#include <QFileInfo>
#include <utility>

EmailDialog::EmailDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EmailDialog)
{
    ui->setupUi(this);
}

EmailDialog::~EmailDialog()
{
    delete ui;
}

void EmailDialog::setEmailAddress(const QString& emailAddress)
{
    ui->lineEditEmailAddress->setText(emailAddress);
}

QString EmailDialog::emailAddress()
{
    return ui->lineEditEmailAddress->text();
}

void EmailDialog::setFileNames(QStringList fileNames)
{
    _fileNames = std::move(fileNames);
}

QStringList EmailDialog::fileNames()
{
    return _fileNames;
}

int EmailDialog::exec()
{
    QIcon icon;
    QPixmap pixmap;

    foreach(QString fileName, _fileNames) {
//        qDebug() << "EmailDialog:" << fileName;
        QFileInfo fileInfo(fileName);
        QString cacheDir = fileInfo.absolutePath() + "/.Cache";
        QFileInfo cacheFileInfo(cacheDir + "/cache_" + fileInfo.fileName());
//        qDebug() << "Pixmap load" << cacheFileInfo.absoluteFilePath() <<
        pixmap.load(cacheFileInfo.absoluteFilePath());
        icon.addPixmap(pixmap);
        QListWidgetItem *item = new QListWidgetItem(icon,fileInfo.fileName());
        ui->listWidget->addItem(item);
    }
    return QDialog::exec();
}
