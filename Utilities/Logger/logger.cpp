#include "logger.h"

#include <QMutexLocker>
#include <QDateTime>

Logger::Logger(QObject* parent)
    : QObject{parent}
{
    m_file.setFileName("app.log");
    if(m_file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        m_stream.setDevice(&m_file);
    }
}

Logger::~Logger()
{
    m_file.close();
}

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

void Logger::log(LogLevel level, const QString& message, const QString& category)
{
    {
        QMutexLocker lock(&m_mutex);
        if(!m_file.isOpen()) return;

        QString levelStr;
        switch(level)
        {
        case Debug:
            levelStr = "DEBUG";
            break;
        case Info:
            levelStr = "INFO";
            break;
        case Warning:
            levelStr = "WARN";
            break;
        case Critical:
            levelStr = "CRIT";
            break;
        }

        QString logLine = QString("[%1] %2 <%3> %4\n")
                              .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
                              .arg(levelStr)
                              .arg(category)
                              .arg(message);

        m_stream << logLine;
        m_stream.flush();
    }
}
