#ifndef GRIDVIEWMANAGER_H
#define GRIDVIEWMANAGER_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QGridLayout);

class GridViewManager : public QWidget
{
    Q_OBJECT
public:
    explicit GridViewManager(QWidget *parent = nullptr);

private:
    QGridLayout* m_gridLayout;
    int m_rows;
    int m_cols;
};

#endif // GRIDVIEWMANAGER_H
