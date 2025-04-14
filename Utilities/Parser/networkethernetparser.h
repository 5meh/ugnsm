#ifndef NETWORKETHERNETPARSER_H
#define NETWORKETHERNETPARSER_H

#include "iparser.h"
#include <QNetworkInterface>

class NetworkInfo;
class INetworkSortStrategy;

class NetworkEthernetParser: public IParser
{
public:
    NetworkEthernetParser(INetworkSortStrategy*, QObject* parent = nullptr);

public:
    void parse() override;
private:
    void parseInterface(const QNetworkInterface& interface, QList<NetworkInfo*>& results);
    QString getIPv4Address(const QNetworkInterface& interface) const;
    QString getNetmask(const QNetworkInterface& interface) const;
    QString getBroadcast(const QNetworkInterface& interface) const;
protected:
    virtual bool validate(QVariant& result, QStringList& warnings) override;
};

Q_DECLARE_METATYPE(QList<NetworkInfo*>)

#endif // NETWORKETHERNETPARSER_H
