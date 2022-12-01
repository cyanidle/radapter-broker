#include "jsondict.h"
#include "../logging/jsonformatterslogging.h"
#include <QJsonObject>

using namespace Formatters;

const QVariant JsonDict::value(const QString& akey, const QVariant &adefault) const
{
    return m_dict.value(akey, adefault);
}

const QVariant JsonDict::operator[](const QString& akey) const
{
    return m_dict.value(akey);
}

QVariant& JsonDict::operator[](const QString& akey)
{
    return m_dict[akey];
}

bool JsonDict::isEmpty() const
{
    return m_dict.isEmpty();
}

QStringList JsonDict::keys(const QString &separator) const
{
    QStringList result{};
    for (const auto &iter: *this) {
        result.append(iter.getFullKey().join(separator));
    }
    return result;
}

bool JsonDict::contains(const QStringList &key) const
{
    if (key.length() == 0) {
        return false;
    }
    if (key.length() == 1 && m_dict.value(key[0]).isValid()) {
        return true;
    }
    return value(key).isValid();
}

const QVariant *JsonDict::recurseTo(const QStringList &fullKey, int ignoreLastKeys) const
{
    QVariantMap* currentDict = const_cast<Dict*>(&m_dict);
    QVariant* current = nullptr;
    int count = fullKey.size();
    for (auto &key : fullKey) {
        if (currentDict->contains(key)) {
            current = &(currentDict->operator[](key));
            if (count > 1) {
                if (current->canConvert<QVariantMap>() || current->canConvert<Dict>()) {
                    currentDict = reinterpret_cast<QVariantMap *>(current->data());
                } else {
                    return nullptr;
                }
            }
        } else {
            return nullptr;
        }
        if (--count <= ignoreLastKeys) {
            break;
        }
    }
    return current;
}

bool JsonDict::isValid(const QStringList& akey) const
{
    return value(akey).isValid();
}

bool JsonDict::isValid(const QString& akey) const
{
    return m_dict.value(akey).isValid();
}

JsonDict::iterator JsonDict::begin()
{
    auto iter = ++(JsonDict::iterator(m_dict.begin(), isEmpty()));
    iter.m_firstRun = false;
    return iter;
}

JsonDict::iterator JsonDict::end()
{
    return JsonDict::iterator(m_dict.end(), isEmpty());
}

JsonDict::const_iterator JsonDict::begin() const
{
    auto iter = ++(JsonDict::const_iterator(m_dict.begin(), isEmpty()));
    iter.m_firstRun = false;
    return iter;
}

JsonDict::const_iterator JsonDict::end() const
{
    return JsonDict::const_iterator(m_dict.end(), isEmpty());
}

QString JsonDict::processWarn(const QStringList &src, const int& index)
{
    QString result;
    result.append(src.constFirst());
    for (int i = 1; i < index;++i){
        result.append(":");
        result.append(src.at(i));
    }
    return result;
}

const QVariant JsonDict::operator[](const QStringList& akey) const
{
    return value(akey);
}

const QVariant JsonDict::value(const QStringList& akey, const QVariant &adefault) const
{
    auto ptr = recurseTo(akey);
    if (ptr) {
        return *ptr;
    } else {
        return adefault;
    }
}

int JsonDict::remove(const QStringList &akey)
{
    if (value(akey).isValid()){
        QVariantMap *dictPtr = &m_dict;
        QVariant* dictVar;
        for (int i = 0; i < akey.length() - 1; ++i) {
            dictVar = &dictPtr->operator[](akey[i]);
            dictPtr = reinterpret_cast<QVariantMap*>(dictVar->data());
        }
        return dictPtr->remove(akey[akey.length() - 1]);
    }
    return 0;
}

QVariant JsonDict::take(const QStringList &akey)
{
    if (value(akey).isValid()){
        QVariantMap *dictPtr = &m_dict;
        QVariant* dictVar;
        for (int i = 0; i < akey.length() - 1; ++i) {
            dictVar = &dictPtr->operator[](akey[i]);
            dictPtr = reinterpret_cast<QVariantMap*>(dictVar->data());
        }
        return dictPtr->take(akey[akey.length() - 1]);
    }
    return QVariant();
}

Dict &JsonDict::data()
{
    return m_dict;
}

const Dict& JsonDict::data() const
{
    return constData();
}

Dict JsonDict::flatten(const QString &separator) const
{
    return m_dict.flatten(separator);
}

void JsonDict::merge(const JsonDict &src)
{
    m_dict = m_dict.merge(src.m_dict);
}

const Dict& JsonDict::constData() const
{
    return m_dict;
}
//! Дает доступ на чтение/запись значения под ключем, разделитель - ":"
QVariant& JsonDict::operator[](const QStringList& akey)
{
    bool disableWarn = false;
    QVariantMap* currentDict = &m_dict;
    QVariant* currentVal = &(currentDict->operator[](akey.constFirst()));
    for (int index = 1; index < (akey.size()); ++index) {
        if (currentVal->isValid()) {
            if (currentVal->canConvert<QVariantMap>()) {
                currentDict = reinterpret_cast<QVariantMap*>(currentVal->data());
                currentVal = &(currentDict->operator[](akey.at(index)));
            } else {
                if (!disableWarn) {
                    jfWarn() << "Attempt to get value by overlapping key: (" << akey << "); real value: ("
                             << processWarn(akey,index) << ")";
                }
                return *currentVal;
            }
        } else {
             currentVal->setValue(QVariantMap{});
             currentDict = reinterpret_cast<QVariantMap*>(currentVal->data());
             currentVal = &(currentDict->operator[](akey.at(index)));
        }
    }
    return *currentVal;
}

QJsonObject JsonDict::toJsonObj() const
{
    return QJsonObject::fromVariantMap(m_dict);
}
JsonDict JsonDict::fromJsonObj(const QJsonObject &json)
{
    return JsonDict(json.toVariantMap());
}

bool JsonDict::operator==(const JsonDict& src) const
{
    return m_dict == src.m_dict;
}

bool JsonDict::operator!=(const JsonDict& src) const
{
    return m_dict != src.m_dict;
}

// Ниже код для итератора

JsonDict::iterator::iterator(QVariantMap::iterator iter, bool isEmpty)
    : m_iter(iter),
      m_firstRun(true),
      m_justReturned(false)
{
    if (isEmpty) {
        m_currentEnd = iter;
    } else {
        m_currentEnd = --iter;
    }
}

QStringList JsonDict::iterator::getCurrentDomain() const
{
    QStringList result;
    for (auto iterPair = m_traverseHistory.cbegin(); iterPair != m_traverseHistory.cend(); ++iterPair) {
        result.append(iterPair->first.key());
    }
    return result;
}

QStringList JsonDict::iterator::getFullKey() const
{
    QStringList result = getCurrentDomain();
    result.append(field());
    return result;
}

const QString& JsonDict::iterator::field() const
{
    return m_iter.key();
}

QVariant& JsonDict::iterator::value() const
{
    return m_iter.value();
}

JsonDict::iterator JsonDict::iterator::operator++(int)
{
    JsonDict::iterator temp = *this;
    ++*this;
    return temp;
}

JsonDict::iterator& JsonDict::iterator::operator++()
{
    if (!m_firstRun && !m_justReturned){
        ++m_iter;
    }
    if (m_iter == m_currentEnd && !m_firstRun){
        auto currAndEnd = m_traverseHistory.pop();
        m_iter = currAndEnd.first;
        m_currentEnd = currAndEnd.second;
        m_justReturned = true; // При возврате на уровень выше, для обеспечения многоуровневнего выхода нужно запрещать автоматическую итерацию
        ++m_iter;
        return ++*this;
    }
    QVariant& currVal = m_iter.value();
    if (currVal.canConvert<QVariantMap>())
    {
        m_traverseHistory.push({m_iter, m_currentEnd});
        QVariantMap* currDict = reinterpret_cast<QVariantMap*>(currVal.data());
        m_justReturned = true; // При возврате на уровень выше, для обеспечения многоуровневнего выхода нужно запрещать автоматическую итерацию
        if (currDict->isEmpty()) {
            auto currAndEnd = m_traverseHistory.pop();
            m_iter = currAndEnd.first;
            m_currentEnd = currAndEnd.second;
            ++m_iter;
            return ++*this;
        }
        m_iter = currDict->begin();
        m_currentEnd = currDict->end();
        return ++*this;
    }
    else{
        m_justReturned = false;
        return *this;
    }
}

// const_iterator code

JsonDict::const_iterator::const_iterator(QVariantMap::const_iterator iter, bool isEmpty)
    : m_iter(iter),
      m_firstRun(true),
      m_justReturned(false)
{
    if (isEmpty) {
        m_currentEnd = iter;
    } else {
        m_currentEnd = --iter;
    }
}

QStringList JsonDict::const_iterator::getCurrentDomain() const
{
    QStringList result;
    for (auto iterPair = m_traverseHistory.cbegin(); iterPair != m_traverseHistory.cend();++iterPair)
    {
        result.append(iterPair->first.key());
    }
    return result;
}

QStringList JsonDict::const_iterator::getFullKey() const
{
    QStringList result = getCurrentDomain();
    result.append(field());
    return result;
}

const QString& JsonDict::const_iterator::field() const
{
    return m_iter.key();
}

const QVariant& JsonDict::const_iterator::value()const
{
    return m_iter.value();
}

JsonDict::const_iterator JsonDict::const_iterator::operator++(int)
{
    JsonDict::const_iterator temp = *this;
    ++*this;
    return temp;
}

JsonDict::const_iterator& JsonDict::const_iterator::operator++()
{
    if (!m_firstRun && !m_justReturned){
        ++m_iter;
    }
    if (m_iter == m_currentEnd && !m_firstRun){
        auto currAndEnd = m_traverseHistory.pop();
        m_iter = currAndEnd.first;
        m_currentEnd = currAndEnd.second;
        m_justReturned = true; // При возврате на уровень выше, для обеспечения многоуровневнего выхода нужно запрещать автоматическую итерацию
        ++m_iter;
        return ++*this;
    }
    const QVariant& currVal = m_iter.value();
    if (currVal.canConvert<QVariantMap>())
    {
        m_traverseHistory.push({m_iter, m_currentEnd});
        const QVariantMap* currDict = reinterpret_cast<const QVariantMap*>(currVal.data());
        m_justReturned = true; // При возврате на уровень выше, для обеспечения многоуровневнего выхода нужно запрещать автоматическую итерацию
        if (currDict->isEmpty()) {
            auto currAndEnd = m_traverseHistory.pop();
            m_iter = currAndEnd.first;
            m_currentEnd = currAndEnd.second;
            ++m_iter;
            return ++*this;
        }
        m_iter = currDict->begin();
        m_currentEnd = currDict->end();
        return ++*this;
    }
    else{
        m_justReturned = false;
        return *this;
    }
}

