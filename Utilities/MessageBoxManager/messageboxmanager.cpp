#include "messageboxmanager.h"

#include <QCheckBox>

MessageBoxManager::MessageBoxManager(QObject* parent)
    : QObject(parent)
{
    m_dialogFlags["SwapWarning"] = true;
    m_dialogFlags["BestNetworkMove"] = true;
}

bool MessageBoxManager::shouldShowDialog(const QString& dialogId)
{
    return m_dialogFlags.value(dialogId, true) && !m_activeDialogs.contains(dialogId);
}

bool MessageBoxManager::isDialogEnabled(const QString& dialogId) const
{
    return m_dialogFlags.value(dialogId, true);
}

void MessageBoxManager::showDialog(
                                    const QString& dialogId,
                                    const QString& title,
                                    const QString& message,
                                    const QString& checkboxText,
                                    QMessageBox::StandardButtons buttons
                                    )
{
    if (!shouldShowDialog(dialogId))
        return;

    QMessageBox* msgBox = new QMessageBox();
    msgBox->setWindowTitle(title);
    msgBox->setText(message);
    msgBox->setStandardButtons(buttons);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);

    if (!checkboxText.isEmpty())
    {
        QCheckBox* checkBox = new QCheckBox(checkboxText, msgBox);
        msgBox->setCheckBox(checkBox);
    }

    m_activeDialogs.insert(dialogId);

    QObject::connect(msgBox, &QMessageBox::finished, [this, dialogId, msgBox](int result) {

        if (msgBox->checkBox() && msgBox->checkBox()->isChecked())
            m_dialogFlags[dialogId] = false;

        m_activeDialogs.remove(dialogId);
        emit dialogFinished(dialogId, static_cast<QMessageBox::StandardButton>(result));
    });

    msgBox->show();
}
