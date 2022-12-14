#ifndef DICT_H
#define DICT_H

#ifndef RADAPTER_SHARED_SRC
#define RADAPTER_SHARED_SRC
#endif

#include <QVariantMap>
#include <initializer_list>

namespace Formatters{
    class RADAPTER_SHARED_SRC Dict;
}
/*!
 * \defgroup Dict Dict
 * \ingroup Core
 * \brief Класс-наследник QVariantMap.
 *
 *  Данный класс не предъявляет требований к структуре данных внутри себя. Поддерживает вложенность
 *  (ее следует реализовывать через вкладывание Radapter::Core::Dict). Для этого существуют конструкторы через
 *  список инициализации, а также метод merge()
 *  @{
 */
class Formatters::Dict : public QVariantMap
{
public:
    explicit Dict(const QVariant &jsonData);
    Dict() : QVariantMap() {}
    Dict(const Dict &src) : QVariantMap(src) {}
    Dict(const std::initializer_list<std::pair<QString, QVariant>> &initializer) : QVariantMap(initializer) {}
    Dict(std::initializer_list<std::pair<QString, QVariant>> &&initializer) : QVariantMap(std::move(initializer)) {}
    Dict(const QVariantMap &jsonDict) : QVariantMap(jsonDict) {}
    Dict(QVariantMap &&src) : QVariantMap(std::move(src)) {}
    Dict(Dict &&src) : QVariantMap(std::move(src)) {}
    Dict &operator=(const Dict &src) {QVariantMap::operator=(src); return *this;}
    Dict &operator=(Dict &&src) {swap(src); return *this;}
    /*!
     *  \defgroup DictAdded Добавленные методы
     *  \ingroup Dict
     *  @{
    */
    //! Приводит словарь к "плоскому" виду.
    /// \todo Кастомизируемый сепаратор.
    ///
    /// **Плоский** вид подразумевает:
    /// \code
    /// {key0:subkey:
    ///     subvalue,
    /// key1:
    ///     value}
    /// \endcode
    Dict flatten(const QString &separator = ":") const;
    /*! Приводит словарь к "вложенному" виду.
    * \todo Кастомизируемый сепаратор.
    *
    * **Вложенный** вид подразумевает:
    * \code
    * {key0:
    *     {subkey:
    *         subvalue},
    * key1:
    *     value}
    * \endcode
    */
    Dict nest(const QString &separator = ":") const;
    //! Объединяет два словаря в один.
    /// Значения под дупликатными ключами перезаписываются значениями под теми же
    /// ключами в словаре src
    /// \param src - Словарь, с которым производистя слияние
    Dict merge(const Dict &src);
    //! Обертка QVariant::value().
    /// Если конвертация возможна возвращает словарь, иначе возвращает пустой Dict
    static Dict fromVariant(const QVariant &var);
    bool isNested() const;
    bool containsDict(const Dict &other) const;
    void insertDict(const Dict &dict);
    /*! @} */ //Added
signals:

private:
    Dict nest(const Dict &json, const QString &separator) const;
    Dict nestFlattened(const Dict &flattenedJson, const QString &separator) const;
};
/*! @} */ //Dict
Q_DECLARE_METATYPE(Formatters::Dict);


#endif // DICT_H
