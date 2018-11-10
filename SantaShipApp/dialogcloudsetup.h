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

    void setS3Access(const QString& value);
    void setS3Secret(const QString& value);
    void setS3Bucket(const QString& value);
    void setS3URL(const QString& value);
    void setEMailFrom(const QString& value);
    void setEMailDomain(const QString& value);
    void setEMailUser(const QString& value);
    void setEMailPassword(const QString& value);
    void setEMailServer(const QString& value);
    void setEMailPort(int value);
    void setEMailTransport(const QString& value);
    void setEMailSubject(const QString& value);
    void setEMailPreamble(const QString& value);
    void setEMailPostamble(const QString& value);

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
