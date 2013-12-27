#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QString>
#include <QTimer>

bool createThumbnail (QString pictureFileName, QString thumbnailFileName);

extern QTimer thumbnailTimer;

#endif // THUMBNAIL_H
