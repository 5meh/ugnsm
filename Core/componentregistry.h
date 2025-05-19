#ifndef COMPONENTREGISTRY_H
#define COMPONENTREGISTRY_H

#include <QHash>
#include <QMetaObject>
#include <QObject>

class ComponentRegistry
{
public:
    template<typename Interface, typename Implementation>
    void registerComponent()
    {
        m_registry.insert(Interface::staticMetaObject.className(),
                          &Implementation::staticMetaObject);
    }

    template<typename Interface>
    Interface* create(QObject* parent = nullptr)
    {
        const QMetaObject* meta = m_registry.value(Interface::staticMetaObject.className());
        return meta ? qobject_cast<Interface*>(meta->newInstance(Q_ARG(QObject*, parent))) : nullptr;
    }

private:
    QHash<QString, const QMetaObject*> m_registry;
};

#endif // COMPONENTREGISTRY_H
