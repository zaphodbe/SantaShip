#ifndef DIALOGCLOUDSETUP_H
#define DIALOGCLOUDSETUP_H

#include <QDialog>

namespace Ui {
class DialogCloudSetup;
}

class DialogCloudSetup : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCloudSetup(QWidget *parent = 0);
    ~DialogCloudSetup();

    void setS3Access(QString value);
    void setS3Secret(QString value);
    void setS3Bucket(QString value);
    void setS3URL(QString value);
    void setEMailFrom(QString value);
    void setEMailDomain(QString value);
    void setEMailUser(QString value);
    void setEMailPassword(QString value);
    void setEMailServer(QString value);
    void setEMailPort(int value);
    void setEMailTransport(QString value);
    void setEMailSubject(QString value);
    void setEMailPreamble(QString value);
    void setEMailPostamble(QString value);

    QString getS3Access(void);
    QString getS3Secret(void);
    QString getS3Bucket(void);
    QString getS3URL(void);
    QString getEMailFrom(void);
    QString getEMailDomain(void);
    QString getEMailUser(void);
    QString getEMailPassword(void);
    QString getEMailServer(void);
    int getEMailPort(void);
    QString getEMailTransport(void);
    QString getEMailSubject(void);
    QString getEMailPreamble(void);
    QString getEMailPostamble(void);

private:
    Ui::DialogCloudSetup *ui;
};

#endif // DIALOGCLOUDSETUP_H
