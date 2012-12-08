#ifndef MYICON_H
#define MYICON_H

#include <QIcon>
#include <QString>

class QMyIcon : public QIcon
{
public:
    QMyIcon(const QString &fileName);
    ~QMyIcon();

private:
    QString _fileName;
};

#endif // MYICON_H
