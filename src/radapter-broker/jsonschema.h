#ifndef RADAPTER_JSONSCHEMA_H
#define RADAPTER_JSONSCHEMA_H

#include <QObject>
#include <QMutex>
#include "JsonFormatters"

namespace Radapter {

class JsonSchema : public QObject
{
    Q_OBJECT
public:
    friend class WorkerMsg;
    template<class T>
    bool isInstance(){
        return qobject_cast<T>(this) != nullptr;
    }
    virtual Formatters::JsonDict prepareMsg(const QVariant &source = QVariant()) const = 0;
    virtual QVariant receive(const Formatters::JsonDict &source) const = 0;
    const QString schemaName() const {
        return metaObject()->className();
    }
protected:
    JsonSchema() = default;
};

} // namespace Radapter

#endif // RADAPTER_JSONSCHEMA_H
