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

    void    addImage(QRectF rect)   {imageList->append(rect);};

signals:

public slots:

private:
    QList<QRectF>    *imageList;

};

#endif // IMAGELAYOUT_H
