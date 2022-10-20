#include "jsonbinding.h"
#include "json-formatters/logging/jsonformatterslogging.h"

using namespace Settings;

QRegExp JsonBinding::sm_matcher = QRegExp("^\\{\\w*\\}$");

JsonBinding::JsonBinding() :
    m_name(),
    m_mappings()
{
}

JsonBinding::JsonBinding(const QString &name, const QVariantMap &json) :
    m_name(name),
    m_mappings()
{
    auto currentJson = Formatters::JsonDict(json);
    for (const auto &iter: currentJson) {
        auto currentMapping = Mapping();
        auto currentKey = iter.getFullKey();
        for (int i = 0; i < currentKey.length(); ++i) {
            auto key = currentKey.at(i);
            if (sm_matcher.indexIn(key) != -1) {
                currentMapping.keyIndexes.insert(key.mid(1,key.length()-2), i);
            }
        }
        auto value = iter.value().toString();
        if (sm_matcher.indexIn(value) != -1) {
            currentMapping.valueName = (value.mid(1,value.length()-2));
        } else {
            currentMapping.valueDefault = iter.value();
        }
        currentMapping.mappedKey = currentKey;
        m_mappings.append(currentMapping);
    }
}

bool JsonBinding::isValid() const
{
    return !m_mappings.isEmpty();
}

QStringList JsonBinding::keys(const QString &separator, const QVariantMap &filter) const
{
    auto result = QStringList();
    for (auto &mapping : m_mappings) {
        auto keysToHave = mapping.keyIndexes.keys();
        if (!targetHasAllFromSource(filter.keys(), keysToHave)) {
            jfWarn() << "Binding (" << m_name << "): keys(): Could not get keys, filter did not provide enough keys";
            jfWarn() << "Binding (" << m_name << "): keys(): Needed: " << keysToHave << "; Filter has: " << filter.keys();
            return QStringList();
        }
        auto unformattedKey = mapping.mappedKey;
        for (auto iter = mapping.keyIndexes.constBegin(); iter != mapping.keyIndexes.constEnd(); ++iter) {
            auto keyFromFilter = filter.value(iter.key()).toString();
            if (keyFromFilter.isEmpty()) {
                jfWarn() << "Binding (" << m_name << "): keys(): Could not get keys, filter provided empty/nonstring key for: " << iter.key();
                return QStringList();
            }
            unformattedKey[iter.value()] = keyFromFilter;
        }
        result.append(unformattedKey.join(separator));
    }
    return result;
}

JsonBinding::Map JsonBinding::parseList(const QVariantList &srcList)
{
    auto result = Map();
    for (auto &srcVal : srcList) {
        auto srcMap = srcVal.toMap();
        auto currentName = srcMap.value("name").toString();
        auto currentJson = srcMap.value("json").toMap();
        result.insert(currentName, JsonBinding(currentName, currentJson));
    }
    return result;
}

QVariantMap JsonBinding::receive(const QVariantMap &msg, const QVariantMap &filter) const
{
    auto result = QVariantMap();
    auto receivedJson = Formatters::JsonDict(msg);
    quint16 mappingsWithoutValueName = 0;
    for (auto &mapping : m_mappings) {
        if (mapping.valueName.isEmpty()) {
            if (++mappingsWithoutValueName >= m_mappings.size()) {
                jfWarn() << "Binding (" << m_name << "): receive(): Could not receive msg, not a single named value!";
            }
            continue;
        }
        auto keysToHave = mapping.keyIndexes.keys();
        if (!targetHasAllFromSource(filter.keys(), keysToHave)) {
            jfWarn() << "Binding (" << m_name << "): receive(): Could not receive msg, filter did not provide enough keys";
            jfWarn() << "Binding (" << m_name << "): receive(): Needed: " << keysToHave << "; Filter has: " << filter.keys();
            return QVariantMap();
        }
        auto unformattedKey = mapping.mappedKey;
        for (auto iter = mapping.keyIndexes.constBegin(); iter != mapping.keyIndexes.constEnd(); ++iter) {
            auto keyFromFilter = filter.value(iter.key()).toString();
            if (keyFromFilter.isEmpty()) {
                jfWarn() << "Binding (" << m_name << "): receive(): Could not receive msg, filter provided empty/nonstring key for: " << iter.key();
                return QVariantMap();
            }
            unformattedKey[iter.value()] = keyFromFilter;
        }
        auto received = receivedJson.value(unformattedKey);
        if (received.isValid()) {
            result.insert(mapping.valueName, received);
        }
    }
    return result;
}

QVariantMap JsonBinding::send(const QVariantMap &mappedMsg) const
{
    auto result = Formatters::JsonDict();
    for (auto &mapping : m_mappings) {
        auto keysToHave = mapping.allKeys();
        if (!targetHasAllFromSource(mappedMsg.keys(), keysToHave)) {
            jfWarn() << "Binding (" << m_name << "): send(): Could not send msg, sent msg did not provide enough keys";
            jfWarn() << "Binding (" << m_name << "): send(): Needed: " << keysToHave << "; Msg has: " << mappedMsg.keys();
            return QVariantMap();
        }
        auto unformattedKey = mapping.mappedKey;
        for (auto iter = mapping.keyIndexes.constBegin(); iter != mapping.keyIndexes.constEnd(); ++iter) {
            auto keyFromFilter = mappedMsg.value(iter.key()).toString();
            if (keyFromFilter.isEmpty()) {
                jfWarn() << "Binding (" << m_name << "): send(): Could not receive msg, msg provided empty/nonstring key for: " << iter.key();
                return QVariantMap();
            }
            unformattedKey[iter.value()] = keyFromFilter;
        }
        if (!mapping.valueName.isEmpty()) {
            result[unformattedKey] = mappedMsg.value(mapping.valueName);
        } else {
            result[unformattedKey] = mapping.valueDefault;
        }
    }
    return result.data();
}



QStringList JsonBinding::Mapping::allKeys() const
{
    auto keysToHave = keyIndexes.keys();
    if (!valueName.isEmpty()) {
        keysToHave.append(valueName);
    }
    return keysToHave;
}
