#include <QApplication>
#include <QDebug>
#include <QImage>
#include <QThread>
#include <QStringList>

#include "thumbnail.h"

#ifndef USE_SCALED_ICON_WIDTH
#define USE_SCALED_ICON_WIDTH 250
#endif

bool createThumbnail (QString pictureFileName, QString thumbnailFileName, QStringList *processList, QTimer *timer)
{
    QImage image;

    bool result = true;
    QThread::sleep(1);
    qDebug() << __FILE__ << __FUNCTION__ << "Loading" << pictureFileName;
    if (image.load(pictureFileName))
    {
        qDebug() << __FILE__ << __FUNCTION__ << "Scaling" << pictureFileName;
        // Create scaled cache image and save it
        image = image.scaledToWidth(USE_SCALED_ICON_WIDTH);

//        qDebug() << __FILE__ << __FUNCTION__ << "Saving" << thumbnailFileName;
        image.save(thumbnailFileName);

        // Remove the file from the processList
        processList->removeOne(pictureFileName);

        // Set timeout to notify app to reload thus refresh thumbnails
        timer->setSingleShot(true);
        timer->start(5000);
    }
    else
    {
        qDebug() << __FILE__ << __FUNCTION__ << "Failed to load" << pictureFileName;
        // Picture load failed
        result = false;
    }

//    qDebug() << __FILE__ << __FUNCTION__ << "Done" << result << thumbnailFileName;
    return result;
}
