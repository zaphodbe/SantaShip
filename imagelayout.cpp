#include "imagelayout.h"

QImageLayoutButton::QImageLayoutButton(QWidget *parent) :
        QPushButton(parent),
        imageList(0)
{
    initLayout();
}

QImageLayoutButton::QImageLayoutButton(const QString &text, QWidget *parent) :
        QPushButton(text, parent),
        imageList(0)
{
    initLayout();
}

QImageLayoutButton::QImageLayoutButton(const QIcon &icon, const QString &text, QWidget *parent) :
        QPushButton(icon, text, parent),
        imageList(0)
{
    initLayout();
}

QImageLayoutButton::~QImageLayoutButton()
{
    delete imageList;
}

void QImageLayoutButton::initLayout()
{
    imageList = new QList<TImage>();
    rect.setCoords(0,0,0,0);
}

void QImageLayoutButton::setRect(QRectF rect)
{
    this->rect = rect;
}

void QImageLayoutButton::setRect(qreal x, qreal y, qreal w, qreal h)
{
    // Just call the other variant so everthing is done in one place
    setRect(QRectF(x,y,w,h));
}

void QImageLayoutButton::addImage(QRectF rect, int flags)
{
    TImage *image = new TImage;
    image->rect = rect;
    image->flags = flags;
    imageList->append(*image);

    // Adjust the bounding rectangle if necessary
    if (rect.left()   < this->rect.left()  ) this->rect.setLeft(rect.left());
    if (rect.right()  > this->rect.right() ) this->rect.setRight(rect.right());
    if (rect.top()    < this->rect.top()   ) this->rect.setTop(rect.top());
    if (rect.bottom() > this->rect.bottom()) this->rect.setBottom(rect.bottom());
}

void QImageLayoutButton::addImage(qreal x, qreal y, qreal w, qreal h, int flags)
{
    // Just call the other variant so everthing is done in one place
    addImage(QRectF(x,y,w,h), flags);
}
