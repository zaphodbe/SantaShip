#ifndef IMAGELAYOUT_H
#define IMAGELAYOUT_H

#include <QObject>
#include <QPushButton>

typedef struct ST_IMAGE
{
    QRectF  rect;
    int     flags;
} TImage;

class QImageLayoutButton : public QPushButton
{
    Q_OBJECT
public:
    QImageLayoutButton (QWidget *parent = 0);
    QImageLayoutButton (const QString & text, QWidget *parent = 0);
    QImageLayoutButton (const QIcon & icon, const QString & text, QWidget *parent = 0);
    ~QImageLayoutButton();

    int     getImageCnt()           {return imageList->length();};
    QRectF  getImageRect(int i)     {return imageList->at(i).rect;};
    int     getImageFlags(int i)    {return imageList->at(i).flags;};

    QRectF  rect;
    void    setRect(QRectF rect);
    void    setRect(qreal x, qreal y, qreal w, qreal h);
    void    addImage(QRectF rect, int flags = 0);
    void    addImage(qreal x, qreal y, qreal w, qreal h, int flags = 0);

    enum    Flags
    {
        CROP_IMAGE      = 0x01
    };

signals:

public slots:

private:
    QList<TImage>    *imageList;
    void              initLayout();

};

#endif // IMAGELAYOUT_H
