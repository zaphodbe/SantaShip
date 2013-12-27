#include <QPixmap>
#include <QDebug>

#include "mythumbnailthread.h"

#ifndef USE_SCALED_ICON_WIDTH
#define USE_SCALED_ICON_WIDTH 250
#endif

MyThumbnailThread::MyThumbnailThread(QObject *parent) :
    QThread(parent)
{
}

void MyThumbnailThread::run()
{
    QPixmap pixmap;
    QString pictureFileName;
    QString cacheFileName;

    qDebug() << __FILE__ << __FUNCTION__ << "Thread created";
    while (true)
    {
        if (!m_inputList.empty())
        {
            pictureFileName = m_inputList.first();
            m_processingList.append(pictureFileName);
            cacheFileName = m_outputList.first();

            qDebug() << __FILE__ << __FUNCTION__ << "Loading" << pictureFileName;
            if (pixmap.load(pictureFileName))
            {
                // Create scaled cache image and save it
                pixmap = pixmap.scaledToWidth(USE_SCALED_ICON_WIDTH);

                qDebug() << __FILE__ << __FUNCTION__ << "Saving" << cacheFileName;
                pixmap.save(cacheFileName);
            }
            else
            {
                // loading the pixmap failed so use a standard icon
//                pixmap = QFileIconProvider::icon(info).pixmap(QSize(USE_SCALED_ICON_WIDTH,USE_SCALED_ICON_WIDTH * 3 / 5));
            }

            // Remove file from list
            m_inputList.removeFirst();
            m_outputList.removeFirst();
            m_processingList.removeOne(pictureFileName);
        }
        msleep(100);
    }
}

bool MyThumbnailThread::queuePicture(QString pictureFileName, QString cacheFileName)
{
    /* check if the file is in the list already */
    /* don't insert it twice */
    if (m_inputList.contains(pictureFileName))
        return false;

    m_inputList.append(pictureFileName);
    m_outputList.append(cacheFileName);
    return true;
}

bool MyThumbnailThread::isQueued(QString fileName)
{
    return (m_inputList.contains(fileName));
}

bool MyThumbnailThread::isProcessing(QString fileName)
{
    return (m_processingList.contains(fileName));
}
