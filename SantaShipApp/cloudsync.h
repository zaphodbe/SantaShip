#pragma once

#include <QThread>
#include <QString>
#include <QFile>
#include <atomic>

class CloudSync : public QThread {
public:
    Q_OBJECT

    Q_PROPERTY(int emailCount READ emailCount WRITE setEmailCount NOTIFY emailCountChanged)
    Q_PROPERTY(int pictureSyncCount READ pictureSyncCount WRITE setPictureSyncCount NOTIFY pictureSyncCountChanged)

signals:
    void emailCountChanged(int);
    void pictureSyncCountChanged(int);

public:
    explicit CloudSync();
    ~CloudSync() override = default;

    void run() override;

    void work();

    int emailCount();
    void setEmailCount(int count);

    int pictureSyncCount();
    void setPictureSyncCount(int count);

// FIXME: these should be in accessors, or better yet, properties so we can have bindings
public:
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

private:
    std::atomic_flag working;
    int m_emailCount;
    int m_pictureSyncCount;

    int syncFiles(const QStringList& files);
    int syncEmails(const QStringList& emails);
    void archiveEMLFile(const QFile& file);
};
