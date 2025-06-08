#ifndef DRAGMANAGER_H
#define DRAGMANAGER_H

#include <QObject>
#include <QPoint>
#include <QSet>
#include <QMutex>

class DragManager : public QObject//TODO:rewok process only one drag at time, remove QSet
{
    Q_OBJECT
public:
    bool startDrag(const QPoint& gridIndex);
    void endDrag(const QPoint& gridIndex);
    bool isDragging() const;

private:
    explicit DragManager(QObject* parent = nullptr);
    QSet<QPoint> m_activeDrags;
    mutable QMutex m_mutex;
};

#endif // DRAGMANAGER_H
