#include "cloudsync.h"
#include "qts3.h"
#include "SmtpMime"

#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

CloudSync::CloudSync() :
  emailPort(0),
  working(),
  m_emailCount(0),
  m_pictureSyncCount(0)
{
}

int CloudSync::emailCount()
{
    return m_emailCount;
}

void CloudSync::setEmailCount(int count)
{
    if (m_emailCount != count) {
        m_emailCount = count;
        emit emailCountChanged(m_emailCount);
    }
}

int CloudSync::pictureSyncCount()
{
    return m_pictureSyncCount;
}

void CloudSync::setPictureSyncCount(int count)
{
    if (m_pictureSyncCount != count) {
        m_pictureSyncCount = count;
        emit pictureSyncCountChanged(m_pictureSyncCount);
    }
}

int CloudSync::syncFiles(const QStringList& files)
{
    int failed = 0;
    QtS3 s3(S3Access, S3Secret);
    QStringList headers;
    headers << "Content-Type: image/jpeg";

    for (int i = 0; i < files.size(); i++) {
        qDebug() << "Processing file:" << files.at(i);
        QFile srcFile(filesDirName + "/" + files.at(i));
        if (S3Bucket == QStringLiteral("FakeBucket")) {
            // fake bucket, don't actually upload, just delay for a moment and delete the file
            qDebug() << "Pretending to upload to fake bucket...";
            sleep(5);
            qDebug() << "Fake upload successful (duh), deleting";
            // Should we randomly fail?
            srcFile.remove();
            setPictureSyncCount(m_pictureSyncCount - 1);
        } else if (srcFile.open(QIODevice::ReadOnly)) {
            QByteArray fileData = srcFile.readAll();
            srcFile.close();
            QtS3Reply<void> putReply = s3.put(S3Bucket.toLocal8Bit(), files.at(i), fileData, headers);
            if (putReply.isSuccess()) {
                qDebug() << "Put successful deleting local copy";
                srcFile.remove();
                setPictureSyncCount(m_pictureSyncCount - 1);
            } else {
                qDebug() << "Put failed, keep for next time through:" << putReply.anyErrorString();
                failed++;
            }
        } else {
            qDebug() << "Error opening file" << srcFile.fileName();
            failed++;
        }
    }

    return failed;
}

void CloudSync::archiveEMLFile(const QFile& file)
{
    QDir emailDir(emailDirName);

    QFileInfo info(file);
    QString fileName = info.fileName();

    // skip non-existent files
    if (!emailDir.exists(fileName)) {
        qCritical() << "Attempt to archive nonexistent eml file: " << fileName;
        return;
    }

    if (!emailDir.exists(".archive")) {
        emailDir.mkdir(".archive");
    }

    QDir archiveDir(emailDir.filePath(".archive"));
    qDebug() << ".archive dir: " << archiveDir;
    QString archiveName = archiveDir.filePath(fileName);
    qDebug() << "Archiving eml file " << fileName << " to " << archiveName;
    emailDir.rename(fileName, archiveName);
}

int CloudSync::syncEmails(const QStringList& emails)
{
    int failed = 0;

    for (int i = 0; i < emails.size(); i++) {
        qDebug() << "Processing email:" << emails.at(i);
        QFile file(emailDirName + "/" + emails.at(i));
        if (file.open(QIODevice::ReadOnly)) {
            // First we need to create an SmtpClient object
            // We will use the Gmail's smtp server (smtp.gmail.com, port 465, ssl)
//            qDebug() << "emailServer" << emailServer;
//            qDebug() << "emailPort" << emailPort;
//            qDebug() << "emailUser" << emailUser;
//            qDebug() << "emailFrom" << emailFrom;
//            qDebug() << "emailDomain" << emailDomain;
//            qDebug() << "emailPassword" << emailPassword;
//            qDebug() << "emailTransport" << emailTransport;

            SmtpClient smtp(emailServer, emailPort, SmtpClient::TcpConnection);
            if (emailTransport == QString("SSL")) {
                smtp.setConnectionType(SmtpClient::SslConnection);
            } else if (emailTransport == QString("TLS")) {
                smtp.setConnectionType(SmtpClient::TlsConnection);
            }

            // We need to set the username (your email address) and password
            // for smtp authentification.

            smtp.setUser(emailUser);
            smtp.setPassword(emailPassword);
            smtp.setName(emailDomain);
            smtp.setAuthMethod(SmtpClient::AuthLogin);

            // Now we create a MimeMessage object. This is the email.

            MimeMessage message;

            EmailAddress sender(emailUser, emailFrom);
            message.setSender(&sender);

            QString rcptTo;
            rcptTo = file.readLine();
            rcptTo.chop(1);

            EmailAddress to(rcptTo, rcptTo);
            message.addRecipient(&to);

            EmailAddress bcc(emailUser, emailUser);
            message.addBcc(&bcc);

            message.setSubject(emailSubject);

            // Now add some text to the email.
            // First we create a MimeText object.

            QString msgText;
            msgText += emailPreamble;
            while (!file.atEnd()) {
                QString destFileName = file.readLine();
                msgText += S3URL + S3Bucket + "/" + destFileName;
            }
            msgText += emailPostamble;

            qDebug() << "Message: " << msgText;
            MimeText text(msgText);

            // Now add it to the mail
            message.addPart(&text);

            // Now we can send the mail
            if (emailDomain == QStringLiteral("fakedomain.com") || emailDomain == QStringLiteral("fakedomain.org")) {
                // Fake sending email
                qDebug() << "NOT sending email to FAKE domain";
                sleep(2);
                qDebug() << "Finished pretending to send fake email";
            } else {
                if (!smtp.connectToHost()) {
                    qDebug() << "Failed to connect to host!";
                    failed ++;
                    break;
                }

                if (!smtp.login()) {
                    qDebug() << "Failed to login!";
                    failed ++;
                    break;
                }

                if (!smtp.sendMail(message)) {
                    qDebug() << "Failed to send mail!";
                    failed ++;
                    break;
                }

                smtp.quit();
                qDebug() << "Successfully staged E-Mail";
            }
            file.close();
            archiveEMLFile(file);
            setEmailCount(m_emailCount - 1);
        } else {
            qDebug() << "Error opening file" << file.fileName();
            failed ++;
        }
    }

    return failed;
}

void CloudSync::work()
{
    // basically a delayed spin-lock, but thread safe
    if (working.test_and_set()) return;

    qDebug() << "CloudSync::work()";

    QDir filesDir(filesDirName);
    QStringList fileList = filesDir.entryList(QDir::Files);
    setPictureSyncCount(fileList.size());

    QDir emailDir(emailDirName);
    QStringList emailList = emailDir.entryList(QDir::Files);
    setEmailCount(emailList.size());

    // First process any pending images to send to the cloud
    bool allSunk = true;
    if (syncFiles(fileList)) allSunk = false;

    // If all the files are uploaded send any pending E-Mails
    if (allSunk && !emailServer.isEmpty() && !emailDomain.isEmpty() && !emailUser.isEmpty() && !emailFrom.isEmpty() && !emailPassword.isEmpty()) {
        qDebug() << "All pictures synced, sending emails";
        syncEmails(emailList);
    }

    working.clear();
}

void CloudSync::run()
{
    qDebug() << "Starting cloud sync thread...";
    while(true) {
        work();
        sleep(30);
    }
}
