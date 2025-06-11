#ifndef DRAGMANAGER_H
#define DRAGMANAGER_H

#include <QObject>

class DragManager : public QObject//TODO:fix semi Singletone later.
{
    Q_OBJECT
public:
    static DragManager* instance();
    explicit DragManager(QObject* parent = nullptr);

    void tryStartDrag();
    void endDrag();
    bool isDragging() const;

signals:
    void dragStarted();
    void dragEnded();
private:
    QAtomicInteger<bool> m_dragActive{false};
};

#endif // DRAGMANAGER_H
