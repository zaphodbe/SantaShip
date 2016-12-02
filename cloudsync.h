#ifndef CLOUDSYNC_H
#define CLOUDSYNC_H

#include <QThread>
#include "SmtpClient-for-Qt/src/SmtpMime"

typedef struct cloudSyncData_s {
    bool    working;

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
} cloudSyncData_t;

class cloudSyncThread : public QThread
{
public:
    cloudSyncThread();
    void run();

    cloudSyncData_t     data;
};

void cloudSyncWork(cloudSyncData_t* data);

#endif // CLOUDSYNC_H
