#ifndef NETWORKETHERNETPARSER_H
#define NETWORKETHERNETPARSER_H

#include "iparser.h"
#include <QNetworkInterface>

class NetworkInfo;

class NetworkEthernetParser: public IParser
{
    Q_OBJECT
    Q_INTERFACES(IParser)
public:
    Q_INVOKABLE NetworkEthernetParser(QObject* parent = nullptr);
    virtual ~NetworkEthernetParser() = default;
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

#endif // NETWORKETHERNETPARSER_H
