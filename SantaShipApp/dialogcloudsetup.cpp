#include "dialogcloudsetup.h"
#include "ui_dialogcloudsetup.h"

DialogCloudSetup::DialogCloudSetup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCloudSetup)
{
    ui->setupUi(this);
}

DialogCloudSetup::~DialogCloudSetup()
{
    delete ui;
}

void DialogCloudSetup::setS3Access(QString value)
{
    ui->lineEditS3Access->setText(value);
}

void DialogCloudSetup::setS3Secret(QString value)
{
    ui->lineEditS3Secret->setText(value);
}

void DialogCloudSetup::setS3Bucket(QString value)
{
    ui->lineEditS3Bucket->setText(value);
}

void DialogCloudSetup::setS3URL(QString value)
{
    ui->lineEditS3URL->setText(value);
}

void DialogCloudSetup::setEMailFrom(QString value)
{
    ui->lineEditEMailFrom->setText(value);
}

void DialogCloudSetup::setEMailDomain(QString value)
{
    ui->lineEditEMailDomain->setText(value);
}

void DialogCloudSetup::setEMailUser(QString value)
{
    ui->lineEditEMailUser->setText(value);
}

void DialogCloudSetup::setEMailPassword(QString value)
{
    ui->lineEditEMailPassword->setText(value);
}

void DialogCloudSetup::setEMailServer(QString value)
{
    ui->lineEditEMailServer->setText(value);
}

void DialogCloudSetup::setEMailPort(int value)
{
    ui->spinBoxEMailPort->setValue(value);
}

void DialogCloudSetup::setEMailTransport(QString value)
{
    ui->comboBoxTransport->setCurrentText(value);
}

void DialogCloudSetup::setEMailSubject(QString value)
{
    ui->lineEditEMailSubject->setText(value);
}

void DialogCloudSetup::setEMailPreamble(QString value)
{
    ui->plainTextEditPreamble->setPlainText(value);
}

void DialogCloudSetup::setEMailPostamble(QString value)
{
    ui->plainTextEditPostamble->setPlainText(value);
}

QString DialogCloudSetup::getS3Access()
{
    return ui->lineEditS3Access->text();
}

QString DialogCloudSetup::getS3Secret()
{
    return ui->lineEditS3Secret->text();
}

QString DialogCloudSetup::getS3Bucket()
{
    return ui->lineEditS3Bucket->text();
}

QString DialogCloudSetup::getS3URL()
{
    return ui->lineEditS3URL->text();
}

QString DialogCloudSetup::getEMailFrom()
{
    return ui->lineEditEMailFrom->text();
}

QString DialogCloudSetup::getEMailDomain()
{
    return ui->lineEditEMailDomain->text();
}

QString DialogCloudSetup::getEMailUser()
{
    return ui->lineEditEMailUser->text();
}

QString DialogCloudSetup::getEMailPassword()
{
    return ui->lineEditEMailPassword->text();
}

QString DialogCloudSetup::getEMailServer()
{
    return ui->lineEditEMailServer->text();
}

int DialogCloudSetup::getEMailPort()
{
    return ui->spinBoxEMailPort->value();
}

QString DialogCloudSetup::getEMailTransport()
{
    return ui->comboBoxTransport->currentText();
}

QString DialogCloudSetup::getEMailSubject()
{
    return ui->lineEditEMailSubject->text();
}

QString DialogCloudSetup::getEMailPreamble()
{
    return ui->plainTextEditPreamble->toPlainText();
}

QString DialogCloudSetup::getEMailPostamble()
{
    return ui->plainTextEditPostamble->toPlainText();
}
