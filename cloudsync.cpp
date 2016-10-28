#include "cloudsync.h"

#include <QDir>
#include <QDebug>
#include "qts3.h"
#include "SmtpClient-for-Qt/src/SmtpMime"

cloudsync::cloudsync()
{

}

void cloudsync::run()
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

        // First process any pending images to send to the cloud
        QStringList files = filesDir.entryList(QDir::Files);
        bool allSunk = true;
        for (int i = 0; i < files.size(); i++) {
            qDebug() << "Processing file:" << files.at(i);
            QFile srcFile(filesDir.absolutePath() + "/" + files.at(i));
            if (srcFile.open(QIODevice::ReadOnly)) {
                QByteArray data = srcFile.readAll();
                QtS3Reply<void> putReply = s3.put(S3Bucket.toLocal8Bit() ,files.at(i) ,data );
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
        if (allSunk) {
            files = emailDir.entryList(QDir::Files);
            for (int i = 0; i < files.size(); i++) {
                qDebug() << "Processing email:" << files.at(i);
                QFile file(emailDir.absolutePath() + "/" + files.at(i));
                if (file.open(QIODevice::ReadOnly)) {
                    // First we need to create an SmtpClient object
                    // We will use the Gmail's smtp server (smtp.gmail.com, port 465, ssl)
                    qDebug() << "emailServer" << emailServer;
                    qDebug() << "emailPort" << emailPort;
                    qDebug() << "emailSender" << emailSender;
                    qDebug() << "emailSenderName" << emailSenderName;
                    qDebug() << "emailPassword" << emailPassword;

                    SmtpClient smtp(emailServer, emailPort, SmtpClient::SslConnection);

                    // We need to set the username (your email address) and password
                    // for smtp authentification.

                    smtp.setUser(emailSender);
                    smtp.setPassword(emailPassword);
                    //smtp.setConnectionType(SmtpClient::TlsConnection);

                    // Now we create a MimeMessage object. This is the email.

                    MimeMessage message;

                    EmailAddress sender(emailSender, emailSenderName);
                    message.setSender(&sender);

                    EmailAddress to("cooley@zaphod.com", "Chris Coley");
                    message.addRecipient(&to);

                    message.setSubject("SmtpClient for Qt - Demo");

                    // Now add some text to the email.
                    // First we create a MimeText object.

                    MimeText text;

                    text.setText("Hi,\nThis is a simple email message.\n");

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
                } else {
                    qDebug() << "Error opening file" << file.fileName();
                }
            }
        }

        sleep(10);
    }
}
