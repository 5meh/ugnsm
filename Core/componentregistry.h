#ifndef COMPONENTREGISTRY_H
#define COMPONENTREGISTRY_H

#include <QHash>
#include <QMetaObject>
#include <QObject>

class ComponentRegistry
{
public:
    template<typename Interface, typename Implementation>
    static void registerComponent()
    {
        m_registry.insert(Interface::staticMetaObject.className(),
                          &Implementation::staticMetaObject);
    }

    template<typename Interface>
    static Interface* create(QObject* parent = nullptr)
    {
        const QMetaObject* meta = m_registry.value(Interface::staticMetaObject.className());
        return meta ? qobject_cast<Interface*>(meta->newInstance(Q_ARG(QObject*, parent))) : nullptr;
    }

private:
    static QHash<QString, const QMetaObject*> m_registry;
};

#endif // COMPONENTREGISTRY_H
