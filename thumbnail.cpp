#include <QApplication>
#include <QDebug>
#include <QImage>
#include <QStringList>

#include "thumbnail.h"

#ifndef USE_SCALED_ICON_WIDTH
#define USE_SCALED_ICON_WIDTH 250
#endif

// Setup a global thumbnailTimer
QTimer thumbnailTimer;

bool createThumbnail (QString pictureFileName, QString thumbnailFileName, QStringList *processList)
{
    QImage image;

    bool result = true;
//    qDebug() << __FILE__ << __FUNCTION__ << "Loading" << pictureFileName;
    if (image.load(pictureFileName))
    {
        // Create scaled cache image and save it
        image = image.scaledToWidth(USE_SCALED_ICON_WIDTH);

//        qDebug() << __FILE__ << __FUNCTION__ << "Saving" << thumbnailFileName;
        image.save(thumbnailFileName);
    }
    else
    {
        // Picture load failed
        result = false;
    }

    // Remove the file from the processList
    processList->removeOne(pictureFileName);

    // Set to timeout after one second to notify app to reload thus refresh thumbnails
    thumbnailTimer.setSingleShot(true);
    thumbnailTimer.start(5000);

//    qDebug() << __FILE__ << __FUNCTION__ << "Done" << result << thumbnailFileName;
    return result;
}
