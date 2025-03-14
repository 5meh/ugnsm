#ifndef LEDINDICATOR_H
#define LEDINDICATOR_H

#include <QFrame>
#include <QColor>

class LedIndicator: public QFrame
{
    Q_OBJECT
public:
    enum class Status{Red, Yellow, Green};//TODO: mb make private late
    Q_ENUM(Status);
    explicit LedIndicator(QWidget* parent = nullptr);
    //TODO: mb make it via Q_PROPERTY
    void setStatus(Status status);
    QString statusToString(Status color);
private:
    Status currentStatus;
    void updateColor();
};

#endif // LEDINDICATOR_H
