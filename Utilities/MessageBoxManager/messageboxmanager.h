#ifndef MESSAGEBOXMANAGER_H
#define MESSAGEBOXMANAGER_H

#include <QObject>
#include <QMessageBox>
#include <QSet>
#include <QMap>

class MessageBoxManager : public QObject
{
    Q_OBJECT
public:
    explicit MessageBoxManager(QObject* parent = nullptr);

    bool shouldShowDialog(const QString& dialogId);

    void showDialog(
        const QString& dialogId,
        const QString& title,
        const QString& message,
        const QString& checkboxText = "",
        QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No
        );

    bool isDialogEnabled(const QString& dialogId) const;

signals:
    void dialogFinished(const QString& dialogId, QMessageBox::StandardButton result);

private:
    QSet<QString> m_activeDialogs;      // Track visible dialogs
    QMap<QString, bool> m_dialogFlags;  // Store "show again" flags
};

#endif // MESSAGEBOXMANAGER_H
