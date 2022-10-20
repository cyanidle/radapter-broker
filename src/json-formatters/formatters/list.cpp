#include "list.h"
using namespace Formatters;

List::List() :
    QVariantList()
{
}

List::List(const QVariantList &src) :
    QVariantList(src)
{
}

List::List(const QStringList &src) :
    QVariantList()
{
    for (auto item = src.cbegin(); item != src.cend(); ++item){
        append(QVariant(*item));
    }
}

List::List(const std::initializer_list<Dict> &jsons)
    : QVariantList()
{
    for (auto item = jsons.begin(); item != jsons.end(); ++item){
        append(QVariant(*item));
    }
}

List::List(const std::initializer_list<QVariant> &values) :
    QVariantList(values)
{
}

QStringList List::toQStringList() const
{
    auto result = QStringList();
    for (auto &item: *this) {
        if (item.canConvert<QString>()) {
            result.append(item.toString());
        }
    }
    return result;
}

Dict List::join() const
{
    auto joinedItems = Dict{};
    for (auto &jsonItem : *this) {
        if (jsonItem.canConvert<Dict>()){
            joinedItems.merge(jsonItem.value<Dict>());
        }
        else if (jsonItem.canConvert<QVariantMap>()) {
            joinedItems.merge(jsonItem.value<QVariantMap>());
        }
    }
    return joinedItems;
}
