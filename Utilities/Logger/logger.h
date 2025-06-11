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
    enum Level
    {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    static Logger& instance();

    void log(Level level, const QString& message, const QString& category = "App");
    void setLogLevel(Level minLevel);

private:
    explicit Logger(QObject* parent = nullptr);
    ~Logger();

    QFile m_file;
    QTextStream m_stream;
    QMutex m_mutex;
    Level m_minLevel = Level::Debug;  // Default log level
};

#endif // LOGGER_H
