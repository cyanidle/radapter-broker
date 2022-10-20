#ifndef JSONBINDING_H
#define JSONBINDING_H

#ifndef RADAPTER_SHARED_SRC
#define RADAPTER_SHARED_SRC
#endif

#include "../formatters/jsondict.h"
#include <QVariantMap>

namespace Settings {
    class JsonBinding;
}

class Settings::JsonBinding
{
public:
    typedef QMap<QString, JsonBinding> Map;
    JsonBinding();
    JsonBinding(const QString &name, const QVariantMap &json);
    QVariantMap receive(const QVariantMap &msg, const QVariantMap &filter = QVariantMap()) const;
    QVariantMap send(const QVariantMap &mappedMsg = QVariantMap()) const;
    QStringList keys(const QString &separator, const QVariantMap &filter = QVariantMap()) const;
    bool isValid() const;
    static Map parseList(const QVariantList &srcList);
private:

    struct Mapping {
        QStringList mappedKey;
        QMap<QString, int> keyIndexes;
        QString valueName; //can be empty
        QStringList allKeys() const;
        QVariant valueDefault;
    };

    template<typename T>
    static bool targetHasAllFromSource(const QList<T> &target, const QList<T> &source) {
        for (const auto &item : source) {
            if (!target.contains(item)) {
                return false;
            }
        }
        return true;
    }

    QString m_name;
    QList<Mapping> m_mappings;
    static QRegExp sm_matcher;
};

#endif // JSONBINDING_H
