#include "logger.h"
#include <QMutexLocker>
#include <QDateTime>
#include <QDir>

Logger::Logger(QObject* parent)
    : QObject{parent}
{
    // Create logs directory if it doesn't exist
    QDir logDir("logs");
    if (!logDir.exists())
        logDir.mkpath(".");

    // Create timestamped log file
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    m_file.setFileName("logs/app_" + timestamp + ".log");

    if(m_file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        m_stream.setDevice(&m_file);
        log(Level::Info, "Logging initialized", "Logger");
    }
}

Logger::~Logger()
{
    log(Level::Info, "Logging terminated", "Logger");
    m_file.close();
}

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

void Logger::setLogLevel(Level minLevel)
{
    {
        QMutexLocker lock(&m_mutex);
        m_minLevel = minLevel;
    }
    log(Level::Info, QString("Log level set to: %1").arg(static_cast<int>(minLevel)), "Logger");
}

void Logger::log(Level level, const QString& message, const QString& category)
{
    QMutexLocker lock(&m_mutex);
    if(!m_file.isOpen() || level < m_minLevel)
        return;

    QString levelStr;
    switch(level)
    {
    case Level::Trace:    levelStr = "TRACE"; break;
    case Level::Debug:    levelStr = "DEBUG"; break;
    case Level::Info:     levelStr = "INFO"; break;
    case Level::Warning:  levelStr = "WARN"; break;
    case Level::Error:    levelStr = "ERROR"; break;
    case Level::Critical: levelStr = "CRITICAL"; break;
    }

    QString logLine = QString("[%1] [%2] <%3> %4\n")
                          .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
                          .arg(levelStr)
                          .arg(category)
                          .arg(message);

    m_stream << logLine;
    m_stream.flush();

#ifdef QT_DEBUG
    qDebug().noquote() << logLine.trimmed();
#endif
}
