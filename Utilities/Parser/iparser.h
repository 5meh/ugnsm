#ifndef IPARSER_H
#define IPARSER_H

#include <QObject>
#include <QVariant>

class IParser : public QObject
{
    Q_OBJECT
public:
    explicit IParser(QObject* parent = nullptr) = default;
    virtual ~IParser() = default;

    virtual void parse() = 0;

signals:
    void parsingCompleted(QVariant result);
    void parsingFailed(QString error);
    void validationErrors(QStringList warnings);

protected:
       virtual bool validate(QVariant& result, QStringList& warnings) = 0;
};

#endif // IPARSER_H
