#ifndef BINDINGSPROVIDER_H
#define BINDINGSPROVIDER_H

#ifndef RADAPTER_SHARED_SRC
#define RADAPTER_SHARED_SRC
#endif

#include "../bindings/jsonbinding.h"
#include "../logging/jsonformatterslogging.h"
#include <QMutex>

namespace Settings {
    class RADAPTER_SHARED_SRC BindingsProvider;
}

class Settings::BindingsProvider : public QObject
{
    Q_OBJECT
public:
    //! If UInt is stored in @source under @valueName (search with @bindingName) is non zero return true (error)
    /// You can set invert to true to check for zero (return true on zero)
    static bool checkIfUInt(const QVariantMap &source, const QString &bindingName, const QString &valueName, bool checkZero = false);
    static BindingsProvider* instance() {
        return &prvInstance();
    }
    static void init(const Settings::JsonBinding::Map &bindings, QObject* parent = nullptr) {
        prvInstance(bindings, parent);
    }
    const Settings::JsonBinding &getBinding(const QString &name);
private:
    static BindingsProvider &prvInstance(const Settings::JsonBinding::Map &bindings = Settings::JsonBinding::Map(), QObject* parent = nullptr) {
        static BindingsProvider provider(bindings, parent);
        return provider;
    }
    BindingsProvider(const Settings::JsonBinding::Map &bindings, QObject* parent = nullptr);
    JsonBinding::Map m_bindings;
    Settings::JsonBinding m_placeholder;
    QMutex m_mutex;
};

#endif // BINDINGSPROVIDER_H
