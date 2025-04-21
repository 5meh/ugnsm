#ifndef IPARSER_H
#define IPARSER_H

#include <QObject>
#include <QVariant>
#include <QtPlugin>

class IParser : public QObject
{
    Q_OBJECT
public:
    explicit IParser(QObject* parent = nullptr);
    virtual ~IParser() = default;

    virtual void parse() = 0;

signals:
    void parsingCompleted(QVariant result);
    void parsingFailed(QString error);
    void validationErrors(QStringList warnings);

protected:
    virtual bool validate(QVariant& result, QStringList& warnings) = 0;
};

#define IParser_iid "com.ugnsm.IParser"
Q_DECLARE_INTERFACE(IParser, IParser_iid)


#endif // IPARSER_H
