#include "cloudsync.h"

#include <QDir>
#include <QDebug>
#include "qts3.h"

cloudSyncThread::cloudSyncThread() :
    QThread()
{

}

void cloudSyncThread::run()
{
    QDir filesDir(filesDirName);
    QDir emailDir(emailDirName);

    // Check for and if necessary create an AWS upload directory
    if (!filesDir.exists())
    {
        // EMail folder doesn't exist so create it
        filesDir.mkpath(filesDirName);
    }

    // Check for and if necessary create an EMail directory
    if (!emailDir.exists())
    {
        // EMail folder doesn't exist so create it
        emailDir.mkpath(emailDirName);
    }

    while (1) {
        QtS3 s3(S3Access,S3Secret);
        QStringList headers;
        headers << "Content-Type: image/jpeg";

        // First process any pending images to send to the cloud
        QStringList files = filesDir.entryList(QDir::Files);
        bool allSunk = true;
        for (int i = 0; i < files.size(); i++) {
            qDebug() << "Processing file:" << files.at(i);
            QFile srcFile(filesDir.absolutePath() + "/" + files.at(i));
            if (srcFile.open(QIODevice::ReadOnly)) {
                QByteArray data = srcFile.readAll();
                QtS3Reply<void> putReply = s3.put(S3Bucket.toLocal8Bit() ,files.at(i) ,data ,headers);
                if (putReply.isSuccess()) {
                    qDebug() << "Put successful deleting local copy";
                    srcFile.close();
                    srcFile.remove();
                } else {
                    qDebug() << "Put failed keep for next time through:" << putReply.anyErrorString();
                    allSunk = false;
                }
            } else {
                qDebug() << "Error opening file" << srcFile.fileName();
            }
        }

        // If all the files are uploaded send any pending E-Mails
        if (allSunk && !emailServer.isEmpty() && !emailDomain.isEmpty() && !emailUser.isEmpty() && !emailFrom.isEmpty() && !emailPassword.isEmpty()) {
            files = emailDir.entryList(QDir::Files);
            for (int i = 0; i < files.size(); i++) {
                qDebug() << "Processing email:" << files.at(i);
                QFile file(emailDir.absolutePath() + "/" + files.at(i));
                if (file.open(QIODevice::ReadOnly)) {
                    // First we need to create an SmtpClient object
                    // We will use the Gmail's smtp server (smtp.gmail.com, port 465, ssl)
                    qDebug() << "emailServer" << emailServer;
                    qDebug() << "emailPort" << emailPort;
                    qDebug() << "emailUser" << emailUser;
                    qDebug() << "emailFrom" << emailFrom;
                    qDebug() << "emailDomain" << emailDomain;
                    qDebug() << "emailPassword" << emailPassword;
                    qDebug() << "emailTransport" << emailTransport;

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

                    message.setSubject("Pictures with Santa on the Magic Ship");

                    // Now add some text to the email.
                    // First we create a MimeText object.

                    QString msgText;
                    msgText += "These are your pictures with Santa on the Magic Ship of Christmas.  They will be available for at least 30 days using the links below.\n\n";
                    while (!file.atEnd()) {
                        QString destFileName = file.readLine();
                        msgText += "https://s3-us-west-1.amazonaws.com/" + S3Bucket + "/" + destFileName;
                    }
                    msgText += "\nThank you\nSanta Claus\nTroop / Crew 799\nMorgan Hill\n";

                    qDebug() << "Message: " << msgText;
                    MimeText text(msgText);

                    // Now add it to the mail
                    message.addPart(&text);

                    // Now we can send the mail

                    if (!smtp.connectToHost()) {
                        qDebug() << "Failed to connect to host!" << endl;
                        continue;
                    }

                    if (!smtp.login()) {
                        qDebug() << "Failed to login!" << endl;
                        continue;
                    }

                    if (!smtp.sendMail(message)) {
                        qDebug() << "Failed to send mail!" << endl;
                        continue;
                    }

                    smtp.quit();

                    qDebug() << "Successfully staged E-Mail";
                    file.close();
                    file.remove();
                } else {
                    qDebug() << "Error opening file" << file.fileName();
                }
            }
        }

        sleep(10);
    }
}
