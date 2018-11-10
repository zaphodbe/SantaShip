#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QString>
#include <QStringList>
#include <QTimer>

bool createThumbnail (const QString& pictureFileName, const QString& thumbnailFileName, QStringList *processList);

#endif // THUMBNAIL_H
