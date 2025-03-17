#ifndef NETWORKINFO_H
#define NETWORKINFO_H

#include <QObject>

class NetworkInfo : public QObject
{
    Q_OBJECT
public:
    explicit NetworkInfo(QObject *parent = nullptr);

signals:
};

#endif // NETWORKINFO_H
