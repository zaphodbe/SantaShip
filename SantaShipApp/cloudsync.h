#ifndef CLOUDSYNC_H
#define CLOUDSYNC_H

#include <QThread>
#include "SmtpMime"

typedef struct cloudSyncData_s {
    bool    working;

    QString filesDirName;
    QString emailDirName;

    QString S3Access;
    QString S3Secret;
    QString S3Bucket;
    QString S3URL;

    QString emailServer;
    QString emailUser;
    QString emailFrom;
    QString emailDomain;
    QString emailPassword;
    int     emailPort;
    QString emailTransport;
    QString emailSubject;
    QString emailPreamble;
    QString emailPostamble;
} cloudSyncData_t;

class cloudSyncThread : public QThread
{
public:
    cloudSyncThread();
    void run();

    cloudSyncData_t     data;
};

void cloudSyncWork(cloudSyncData_t* data);
int cloudSyncFilesWork(cloudSyncData_t* data, QStringList &files);
int cloudSyncEmailsWork(cloudSyncData_t* data, QStringList &emails);

#endif // CLOUDSYNC_H
