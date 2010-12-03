#include "imagelayout.h"

QImageLayoutButton::QImageLayoutButton(QWidget *parent) :
        QPushButton(parent),
        imageList(0)
{
    imageList = new QList<QRectF>();
}

QImageLayoutButton::QImageLayoutButton(const QString &text, QWidget *parent) :
        QPushButton(text, parent),
        imageList(0)
{
    imageList = new QList<QRectF>();
}

QImageLayoutButton::QImageLayoutButton(const QIcon &icon, const QString &text, QWidget *parent) :
        QPushButton(icon, text, parent),
        imageList(0)
{
    imageList = new QList<QRectF>();
}

QImageLayoutButton::~QImageLayoutButton()
{
    delete imageList;
}
