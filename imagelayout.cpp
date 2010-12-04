#include "imagelayout.h"

QImageLayoutButton::QImageLayoutButton(QWidget *parent) :
        QPushButton(parent),
        imageList(0)
{
    imageList = new QList<QRectF>();
    rect = QRectF(0,0,0,0);
}

QImageLayoutButton::QImageLayoutButton(const QString &text, QWidget *parent) :
        QPushButton(text, parent),
        imageList(0)
{
    imageList = new QList<QRectF>();
    rect = QRectF(0,0,0,0);
}

QImageLayoutButton::QImageLayoutButton(const QIcon &icon, const QString &text, QWidget *parent) :
        QPushButton(icon, text, parent),
        imageList(0)
{
    imageList = new QList<QRectF>();
    rect = QRectF(0,0,0,0);
}

QImageLayoutButton::~QImageLayoutButton()
{
    delete imageList;
}

void QImageLayoutButton::addImage(QRectF rect)
{
    imageList->append(rect);
    // Adjust the bounding rectangle
    if (rect.left()   < this->rect.left()  ) this->rect.setLeft(rect.left());
    if (rect.right()  > this->rect.right() ) this->rect.setRight(rect.right());
    if (rect.top()    < this->rect.top()   ) this->rect.setTop(rect.top());
    if (rect.bottom() > this->rect.bottom()) this->rect.setBottom(rect.bottom());
}
