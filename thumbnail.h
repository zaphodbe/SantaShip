#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QString>
#include <QStringList>
#include <QTimer>

bool createThumbnail (QString pictureFileName, QString thumbnailFileName, QStringList *processList, QTimer *timer);

#endif // THUMBNAIL_H
