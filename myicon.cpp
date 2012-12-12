#include "myicon.h"

#include <QDebug>
#include <QPixmap>

#ifndef USE_SCALED_ICON_WIDTH
#define USE_SCALED_ICON_WIDTH 250
#endif

QMyIcon::QMyIcon(const QString &fileName)
{
    // Use a scaled pixmap for the Icon instead of the raw file
    QPixmap pixmap(fileName);
#if USE_SCALED_ICON_WIDTH
    addPixmap(pixmap.scaledToWidth(USE_SCALED_ICON_WIDTH));
#else
    addPixmap(pixmap);
#endif // USE_SCALED_ICON_WIDTH

    _fileName = fileName;
//    qDebug() << __FILE__ << __FUNCTION__ << _fileName;
}

QMyIcon::~QMyIcon()
{
//    qDebug() << __FILE__ << __FUNCTION__ << _fileName;
}
