#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QSpinBox)
QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)

class SettingsManager;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    void loadSettings();
    void applySettings();

signals:
    void settingsChanged();

public slots:
    void accept() override;
    void reject() override;

private:
    void setupUI();
    void createConnections();

    QComboBox* m_updateStrategyCombo;
    QComboBox* m_sortStrategyCombo;
    QSpinBox* m_updateIntervalSpin;
    QCheckBox* m_showBestNetworkWarningCheck;
    QComboBox* m_dataUnitsCombo;
    QSpinBox* m_decimalPrecisionSpin;
    QCheckBox* m_autoRefreshCheck;
    QComboBox* m_bestNetworkCriteriaCombo;
    QDialogButtonBox* buttons;
};

#endif // SETTINGSDIALOG_H
