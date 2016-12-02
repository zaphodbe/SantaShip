#include "cloudsync.h"

#include <QDir>
#include <QDebug>
#include "qts3.h"

void cloudSyncWork (cloudSyncData_t* data)
{
    //qDebug() << "cloudSyncWork";

    QDir filesDir(data->filesDirName);
    QDir emailDir(data->emailDirName);
    QtS3 s3(data->S3Access,data->S3Secret);
    QStringList headers;
    headers << "Content-Type: image/jpeg";

    // First process any pending images to send to the cloud
    QStringList files = filesDir.entryList(QDir::Files);
    bool allSunk = true;
    for (int i = 0; i < files.size(); i++) {
        qDebug() << "Processing file:" << files.at(i);
        QFile srcFile(filesDir.absolutePath() + "/" + files.at(i));
        if (srcFile.open(QIODevice::ReadOnly)) {
            QByteArray fileData = srcFile.readAll();
            QtS3Reply<void> putReply = s3.put(data->S3Bucket.toLocal8Bit() ,files.at(i) ,fileData ,headers);
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
    if (allSunk && !data->emailServer.isEmpty() && !data->emailDomain.isEmpty() && !data->emailUser.isEmpty() && !data->emailFrom.isEmpty() && !data->emailPassword.isEmpty()) {
        files = emailDir.entryList(QDir::Files);
        for (int i = 0; i < files.size(); i++) {
            qDebug() << "Processing email:" << files.at(i);
            QFile file(emailDir.absolutePath() + "/" + files.at(i));
            if (file.open(QIODevice::ReadOnly)) {
                // First we need to create an SmtpClient object
                // We will use the Gmail's smtp server (smtp.gmail.com, port 465, ssl)
//                qDebug() << "emailServer" << data->emailServer;
//                qDebug() << "emailPort" << data->emailPort;
//                qDebug() << "emailUser" << data->emailUser;
//                qDebug() << "emailFrom" << data->emailFrom;
//                qDebug() << "emailDomain" << data->emailDomain;
//                qDebug() << "emailPassword" << data->emailPassword;
//                qDebug() << "emailTransport" << data->emailTransport;

                SmtpClient smtp(data->emailServer, data->emailPort, SmtpClient::TcpConnection);
                if (data->emailTransport == QString("SSL")) {
                    smtp.setConnectionType(SmtpClient::SslConnection);
                } else if (data->emailTransport == QString("TLS")) {
                    smtp.setConnectionType(SmtpClient::TlsConnection);
                }

                // We need to set the username (your email address) and password
                // for smtp authentification.

                smtp.setUser(data->emailUser);
                smtp.setPassword(data->emailPassword);
                smtp.setName(data->emailDomain);
                smtp.setAuthMethod(SmtpClient::AuthLogin);

                // Now we create a MimeMessage object. This is the email.

                MimeMessage message;

                EmailAddress sender(data->emailUser, data->emailFrom);
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
                    msgText += "https://s3-us-west-1.amazonaws.com/" + data->S3Bucket + "/" + destFileName;
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

    data->working = false;
}

cloudSyncThread::cloudSyncThread() :
    QThread()
{
}

void cloudSyncThread::run()
{
    while(1) {
        cloudSyncWork(&data);

        sleep(30);
    }
}
