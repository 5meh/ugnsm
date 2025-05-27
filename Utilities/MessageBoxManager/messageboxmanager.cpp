#include "messageboxmanager.h"

#include <QCheckBox>

MessageBoxManager::MessageBoxManager(QObject* parent)
    : QObject(parent)
{
    m_dialogFlags["SwapWarning"] = true;
    m_dialogFlags["BestNetworkMove"] = true;
}

bool MessageBoxManager::shouldShowDialog(const QString& dialogId)//TODO:mb make prive an move to showDIalog method
{
    for (const auto& [blocker, blockedDialogs] : m_blockingRelations.asKeyValueRange())
        if (m_activeDialogs.contains(blocker) && blockedDialogs.contains(dialogId))
            return false;

    return m_dialogFlags.value(dialogId, true) && !m_activeDialogs.contains(dialogId);
}

void MessageBoxManager::addBlockingRelationship(const QString& blockingDialog, const QString& blockedDialog)
{
    QMutexLocker locker(&m_mutex);
    m_blockingRelations[blockingDialog].insert(blockedDialog);
}

void MessageBoxManager::clearBlockingRelationship(const QString& dialogId)
{
    QMutexLocker locker(&m_mutex);
    m_blockingRelations[dialogId].clear();
}

bool MessageBoxManager::isDialogEnabled(const QString& dialogId) const
{
    QMutexLocker locker(&m_mutex);
    return m_dialogFlags.value(dialogId, true);
}

QMessageBox::StandardButtons MessageBoxManager::showDialog(
    const QString& dialogId,
    const QString& title,
    const QString& message,
    const QString& checkboxText,
    bool isModal,
    QMessageBox::StandardButtons buttons
    )
{
    QMutexLocker locker(&m_mutex);

    if(!shouldShowDialog(dialogId))
        return QMessageBox::Ok;

    m_activeDialogs.insert(dialogId);

    QMessageBox* msgBox = new QMessageBox();
    msgBox->setWindowTitle(title);
    msgBox->setText(message);
    msgBox->setStandardButtons(buttons);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setModal(isModal);

    if (!checkboxText.isEmpty())
    {
        QCheckBox* checkBox = new QCheckBox(checkboxText, msgBox);
        msgBox->setCheckBox(checkBox);
    }

    QMessageBox::StandardButton result = QMessageBox::NoButton;
    result = static_cast<QMessageBox::StandardButton>(msgBox->exec());
    m_activeDialogs.remove(dialogId);

    if (msgBox->checkBox() && msgBox->checkBox()->isChecked())
        m_dialogFlags[dialogId] = false;

    return result;
    //emit dialogFinished(dialogId, static_cast<QMessageBox::StandardButton>(result));//TODO:mb remove
}
