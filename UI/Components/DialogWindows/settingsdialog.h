#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>

class QComboBox;
class QSpinBox;
class QCheckBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    void loadSettings();
    void applySettings();

public slots:
    void accept() override;
    void reject() override;

private:
    void setupUI();
    void createConnections();

    QSettings m_settings;

    QComboBox* m_sortStrategyCombo;
    QSpinBox* m_updateIntervalSpin;
    QSpinBox* m_gridRowsSpin;
    QSpinBox* m_gridColsSpin;
    QCheckBox* m_showBestNetworkWarningCheck;
};

#endif // SETTINGSDIALOG_H
