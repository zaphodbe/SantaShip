#ifndef IMAGELAYOUT_H
#define IMAGELAYOUT_H

#include <QObject>
#include <QPushButton>

class QImageLayoutButton : public QPushButton
{
    Q_OBJECT
public:
    QImageLayoutButton (QWidget *parent = 0);
    QImageLayoutButton (const QString & text, QWidget *parent = 0);
    QImageLayoutButton (const QIcon & icon, const QString & text, QWidget *parent = 0);
    ~QImageLayoutButton();

    int     getImageCnt()           {return imageList->length();};
    QRectF  getImageRect(int i)     {return imageList->at(i);};

    QRectF  rect;
    void    setRect(QRectF rect);
    void    setRect(qreal x, qreal y, qreal w, qreal h);
    void    addImage(QRectF rect);
    void    addImage(qreal x, qreal y, qreal w, qreal h);

signals:

public slots:

private:
    QList<QRectF>    *imageList;
    void              initLayout();

};

#endif // IMAGELAYOUT_H
