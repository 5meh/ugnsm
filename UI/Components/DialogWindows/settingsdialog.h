#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QSpinBox)
QT_FORWARD_DECLARE_CLASS(QCheckBox)

class SettingsManager;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(SettingsManager* settingsManager, QWidget* parent = nullptr);

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

    SettingsManager* m_settingsManager;
    QComboBox* m_sortStrategyCombo;
    QSpinBox* m_updateIntervalSpin;
    QSpinBox* m_gridRowsSpin;
    QSpinBox* m_gridColsSpin;
    QCheckBox* m_showBestNetworkWarningCheck;
    QComboBox* m_dataUnitsCombo;
    QSpinBox* m_decimalPrecisionSpin;
    QCheckBox* m_autoRefreshCheck;
    QComboBox* m_bestNetworkCriteriaCombo;
};

#endif // SETTINGSDIALOG_H
