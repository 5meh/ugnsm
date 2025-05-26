#ifndef MESSAGEBOXMANAGER_H
#define MESSAGEBOXMANAGER_H

#include <QObject>
#include <QMessageBox>
#include <QSet>
#include <QMap>
#include <QHash>
#include <QMutexLocker>

class MessageBoxManager : public QObject
{
    Q_OBJECT
public:
    explicit MessageBoxManager(QObject* parent = nullptr);

    void addBlockingRelationship(const QString& blockingDialog, const QString& blockedDialog);
    void clearBlockingRelationship(const QString& dialogId);

    QMessageBox::StandardButtons showDialog(
        const QString& dialogId,
        const QString& title,
        const QString& message,
        const QString& checkboxText = "",
        bool isModal = true,
        QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No
        );

    bool isDialogEnabled(const QString& dialogId) const;

signals:
    void dialogFinished(const QString& dialogId, QMessageBox::StandardButton result);

private:
    bool shouldShowDialog(const QString& dialogId);

    mutable QMutex m_mutex;
    QSet<QString> m_activeDialogs;      // Track visible dialogs
    QMap<QString, bool> m_dialogFlags;  // Store "show again" flags
    QHash<QString, QSet<QString>> m_blockingRelations;
};

#endif // MESSAGEBOXMANAGER_H
