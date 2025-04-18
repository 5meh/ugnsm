#include "iparser.h"

#include "networkethernetparser.h"

IParser::IParser(QObject *parent)
    : QObject{parent}
{

}

IParser *IParser::create(ParserType type, INetworkSortStrategy *sorter, QObject *parent)
{
    switch(type)
    {
    case ParserType::Ethernet:
        return new NetworkEthernetParser(sorter, parent);
    // case ParserType::Wireless:
    //     return new NetworkWirelessParser(sorter, parent);
    default:
        return new NetworkEthernetParser(sorter, parent);
    }
}
