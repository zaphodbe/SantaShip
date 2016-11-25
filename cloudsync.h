#ifndef CLOUDSYNC_H
#define CLOUDSYNC_H

#include <QThread>
#include "SmtpClient-for-Qt/src/SmtpMime"

class cloudSyncThread : public QThread
{
public:
    cloudSyncThread();
    void run();

    QString filesDirName;
    QString emailDirName;

    QString S3Access;
    QString S3Secret;
    QString S3Bucket;

    QString emailServer;
    QString emailUser;
    QString emailFrom;
    QString emailDomain;
    QString emailPassword;
    int     emailPort;
    QString emailTransport;
};

#endif // CLOUDSYNC_H
