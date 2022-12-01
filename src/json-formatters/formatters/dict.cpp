#include "dict.h"

using namespace Formatters;

Dict::Dict() : QVariantMap()
{
}

bool Dict::isNested() const
{
    for (auto keyVal = cbegin(); keyVal != cend(); ++keyVal) {
        if (keyVal.value().canConvert<QVariantMap>()) {
            return true;
        }
    }
    return false;;
}

Dict Dict::fromVariant(const QVariant &var)
{
    return var.canConvert<Dict>() ? var.value<Dict>() : Dict{};
}

Dict::Dict(const QVariantMap &jsonDict)
    : QVariantMap(jsonDict)
{
}

Dict::Dict(const QVariant &jsonData)
    : QVariantMap(jsonData.toMap())
{
}

Dict::Dict(const std::initializer_list<std::pair<QString,QVariant>> &initializer)
{
    for (auto pair = initializer.begin(); pair != initializer.end(); ++pair) {
         insert(pair->first, pair->second);
    }
}

Dict Dict::flatten(const QString &separator) const
{
    if ((count() == 1)
            && !first().canConvert<QVariantMap>())
    {
        return Dict();
    }

    auto flattenedJson = Dict{};
    for (auto jsonItem = begin();
         jsonItem != end();
         jsonItem++)
    {
        if (jsonItem.value().canConvert<QVariantMap>()) {
            auto nextLevelMap = Dict(jsonItem.value()).flatten(separator);
            for (auto nextLevelItem = nextLevelMap.begin();
                 nextLevelItem != nextLevelMap.end();
                 nextLevelItem++)
            {
                auto flattenedName = QStringList{ jsonItem.key(), nextLevelItem.key() }.join(separator);
                flattenedJson.insert(flattenedName, nextLevelItem.value());
            }
        } else {
            flattenedJson.insert(jsonItem.key(), jsonItem.value());
        }
    }
    return flattenedJson;
}

Dict Dict::nest(const QString &separator) const
{
    auto nestedJson = nest(*this,separator);
    return nestedJson;
}

Dict Dict::nest(const Dict &json,const QString &separator) const
{
    if (json.isEmpty()){
        return json;
    }
    if (json.firstKey().contains(separator)) {
        auto nestedJson = nestFlattened(json,separator);
        return nestedJson;
    }

    auto nestedJson = Dict{};
    for (auto jsonItem = json.begin();
         jsonItem != json.end();
         jsonItem++)
    {
        if (!jsonItem.value().canConvert<QVariantMap>()) {
            nestedJson.insert(jsonItem.key(), jsonItem.value());
            continue;
        }

        auto nestedValues = nest(Dict(jsonItem.value()), separator);
        nestedJson.insert(jsonItem.key(), nestedValues);
    }
    return nestedJson;
}



Dict Dict::nestFlattened(const Dict &flattenedJson, const QString &separator) const
{
    auto nestedJson = Dict{};
    for (auto sourceItem = flattenedJson.begin();
         sourceItem != flattenedJson.end();
         sourceItem++)
    {
        auto nestedKeys = sourceItem.key().split(separator);
        auto nestedItem = Dict{{ nestedKeys.takeLast(), sourceItem.value()}};
        for (auto levelKey = nestedKeys.rbegin();
             levelKey != nestedKeys.rend();
             levelKey++)
        {
            nestedItem = Dict{{ *levelKey, QVariant(nestedItem)}};
        }
        nestedJson.merge(nestedItem);
    }
    return nestedJson;
}

Dict Dict::merge(const Dict &src)
{
    auto mergedJson = Dict{};
    bool needFullMerge = true;
    for (auto sourceItem = begin();
         sourceItem != end();
         sourceItem++)
    {
        if (!src.contains(sourceItem.key())) {
            continue;
        }

        auto itemToMerge = src.value(sourceItem.key());
        bool mergedItemIsMap = itemToMerge.canConvert<QVariantMap>();
        bool sourceItemIsMap = sourceItem.value().canConvert<QVariantMap>();
        if (sourceItemIsMap && mergedItemIsMap) {
            auto mergedValues = Dict(sourceItem.value()).merge(Dict(itemToMerge));
            mergedJson.insert(sourceItem.key(), mergedValues);
        } else if (sourceItemIsMap) {
            mergedJson.insert(sourceItem.key(), sourceItem.value());
        } else {
            mergedJson.insert(sourceItem.key(), itemToMerge);
        }
        needFullMerge = false;
    }
    if (needFullMerge) {
        mergedJson.insertDict(*this);
        mergedJson.insertDict(src);
        *this = mergedJson;
    } else {
        insertDict(mergedJson);
    }
    return *this;
}


void Dict::insertDict(const Dict &dict)
{
    for (auto iter = dict.constBegin(); iter != dict.constEnd(); ++iter) {
        insert(iter.key(), iter.value());
    }
}

bool Dict::containsDict(const Dict &other) const
{
    if (count() < other.count()) {
        return false;
    }
    bool doesContains = true;
    for (auto otherItem = other.begin();
         otherItem != other.end();
         otherItem++)
    {
        auto thisItem = value(otherItem.key());
        if (!thisItem.isValid() || (thisItem != otherItem.value())) {
            doesContains = false;
            break;
        }
    }
    return doesContains;
}
