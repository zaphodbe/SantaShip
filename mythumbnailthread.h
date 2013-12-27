#ifndef MYTHUMBNAILTHREAD_H
#define MYTHUMBNAILTHREAD_H

#include <QStringList>
#include <QThread>

class MyThumbnailThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThumbnailThread(QObject *parent = 0);

    bool queuePicture(QString pictureFileName, QString cacheFileName);
    bool isQueued(QString fileName);
    bool isProcessing(QString fileName);

protected:
    void run();

signals:

public slots:

private:
    QStringList          m_inputList;
    QStringList          m_outputList;
    QStringList          m_processingList;
};

#endif // MYTHUMBNAILTHREAD_H
