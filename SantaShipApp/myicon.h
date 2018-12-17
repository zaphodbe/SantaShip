#ifndef MYICON_H
#define MYICON_H

#include <QIcon>
#include <QString>
#include <QFileInfo>

class QMyIcon : public QIcon
{
public:
    QMyIcon(const QFileInfo &info);
    ~QMyIcon();

private:
    QString _fileName;
};

#endif // MYICON_H
