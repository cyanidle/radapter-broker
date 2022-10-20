#include "bindingsprovider.h"

using namespace Settings;

BindingsProvider::BindingsProvider(const Settings::JsonBinding::Map &bindings, QObject* parent) :
    QObject(parent),
    m_bindings(bindings),
    m_placeholder(),
    m_mutex()
{
}

const JsonBinding &BindingsProvider::getBinding(const QString &name)
{
    m_mutex.lock();
    if (!m_bindings.contains(name)) {
        jfWarn() << "(WARNING) Bindings provider: No binding found, returning default for name: " << name;
        m_mutex.unlock();
        return m_placeholder;
    } else {
        if (!m_bindings[name].isValid()) {
            jfWarn() << "(WARNING) Bindings provider: Binding is invalid for name: " << name;
        }
        m_mutex.unlock();
        return m_bindings[name];
    }
}


bool BindingsProvider::checkIfUInt(const QVariantMap &source, const QString &bindingName, const QString &valueName, bool checkZero)
{
    auto osmosStatusBinding = instance()->getBinding(bindingName);
    auto statusVal = osmosStatusBinding
                         .receive(source)
                         .value(valueName);
    if (!statusVal.isValid()) {
        return false;
    }
    bool ok;
    auto status = statusVal.toUInt(&ok);
    if (!ok) {
        jfWarn() << "Incorrect status received: " << statusVal;
        return false;
    }
    if (status == 0) {
        return checkZero;
    } else {
        return !checkZero;
    }
}
