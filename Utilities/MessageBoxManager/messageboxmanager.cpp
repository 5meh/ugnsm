#include "messageboxmanager.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QThread>

MessageBoxManager::MessageBoxManager(QObject* parent)
    : QObject(parent)
{
    m_dialogFlags["SwapWarning"] = true;
    m_dialogFlags["BestNetworkMove"] = true;
}

bool MessageBoxManager::shouldShowDialog(const QString& dialogId)//TODO:mb make prive an move to showDIalog method
{
    QMutexLocker locker(&m_mutex);
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
    QMessageBox::StandardButtons buttons,
    int timeoutMs
    )//TODO:mb later somehow separate warning, info, critical message boxes
{
    bool shouldShow = false;
    {
        QMutexLocker locker(&m_mutex);
        shouldShow = shouldShowDialog(dialogId);
        if (shouldShow)
            m_activeDialogs.insert(dialogId);
    }


    if (!shouldShow)
        return QMessageBox::Ok;

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

    msgBox->show();
    QElapsedTimer timer;

    while (msgBox->isVisible())
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);

        if (timeoutMs > 0 && timer.elapsed() > timeoutMs)
        {
            msgBox->reject();
            break;
        }

        QThread::msleep(50);
    }

    bool checkBoxChecked = msgBox->checkBox() && msgBox->checkBox()->isChecked();
    auto result = static_cast<QMessageBox::StandardButton>(msgBox->result());

    {
        QMutexLocker locker(&m_mutex);
        m_activeDialogs.remove(dialogId);
        if (checkBoxChecked)
            m_dialogFlags[dialogId] = false;
    }

    return result;
}
