#ifndef CLOUDSYNC_H
#define CLOUDSYNC_H

#include <QThread>

class cloudsync : public QThread
{
public:
    cloudsync();
    void run();

    QString filesDirName;
    QString emailDirName;

    QString S3Access;
    QString S3Secret;
    QString S3Bucket;

    QString emailServer;
    QString emailSender;
    QString emailSenderName;
    QString emailPassword;
    int     emailPort;
};

#endif // CLOUDSYNC_H
