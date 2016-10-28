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
    void setEMailName(QString value);
    void setEMailUser(QString value);
    void setEMailPassword(QString value);
    void setEMailServer(QString value);
    void setEMailPort(int value);

    QString getS3Access(void);
    QString getS3Secret(void);
    QString getS3Bucket(void);
    QString getEMailName(void);
    QString getEMailUser(void);
    QString getEMailPassword(void);
    QString getEMailServer(void);
    int getEMailPort(void);

private:
    Ui::DialogCloudSetup *ui;
};

#endif // DIALOGCLOUDSETUP_H
