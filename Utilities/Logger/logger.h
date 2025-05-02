#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>

#include <QFile>
#include <QTextStream>
#include <QMutex>

class Logger : public QObject
{
    Q_OBJECT
public:
    enum LogLevel
    {
        Debug,
        Info,
        Warning,
        Critical
    };

    static Logger& instance();

    void log(LogLevel level, const QString& message, const QString& category = "App");

private:
    explicit Logger(QObject* parent = nullptr);
    ~Logger();


    QFile m_file;
    QTextStream m_stream;
    QMutex m_mutex;
};

#endif // LOGGER_H
