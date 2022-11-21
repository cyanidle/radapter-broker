#ifndef JsonDict_H
#define JsonDict_H

#ifndef RADAPTER_SHARED_SRC
#define RADAPTER_SHARED_SRC
#endif

#include <QObject>
#include <QDateTime>
#include <QJsonDocument>
#include <QStack>
#include "dict.h"
#include "list.h"

namespace Formatters{
    class RADAPTER_SHARED_SRC JsonDict;
}

/*!
 * \defgroup JsonDict JsonDict
 * \ingroup Core
 * \ingroup Dict
 * \brief Основной тип сообщения.
 *
 *  Является основным типом данных, с помощью которого происходит обмен информацией между Worker`ами.
 *  Подразумевает глубоко вложенную структуру. Все запросы формируются рекурсивно, в соответствии с вложенностью JsonDict.
 *  (см. Radapter::Redis::Entry)
 * 
 * \code 
 * Core::JsonDict{
 * {domain}:
 *      {entity}:
 *           {field}:
 *                value0
 *      {entity}:
 *           {field}:
 *                value1
 * }
 * \endcode
 * 
 *  @{
 */
class Formatters::JsonDict
{
    friend Formatters::Dict;
public:

    explicit JsonDict(const QVariant& src) : m_dict(src) {}
    JsonDict(const Dict& src = Dict{}) : m_dict(src) {}
    JsonDict(const QVariantMap& src) : m_dict(src) {}
    JsonDict(const std::initializer_list<std::pair<QString, QVariant>> &initializer) : m_dict(initializer) {}
    JsonDict(std::initializer_list<std::pair<QString, QVariant>> &&initializer) : m_dict(std::move(initializer)) {}
    JsonDict(Dict&& src) : m_dict(std::move(src)) {}
    JsonDict(QVariantMap&& src) : m_dict(std::move(src)) {}
    //! Функция доступа к вложенным элементам.
    /// \warning Попытка доступа к несуществующему ключу создает пустое значение в нем,
    /// не повлияет на данные, но стоит быть внимательным (возвращает пустой QVariant())
    ///
    /// \warning Если ключ неполный, внутри QVariant будет словарь (субдомен)
    ///
    /// \warning Если ключ пытается получить значение "ниже" уже существующего **значения**, то онок
    /// не продолжит создавать вложенные словари, а выведет предупреждение в консоль и вернет само
    /// перекрываемое значение
    QVariant& operator[](const QStringList& akey);
    QVariant& operator[](const QString& akey);
    const QVariant operator[](const QStringList& akey) const;
    const QVariant operator[](const QString& akey) const;
    //! Не создает веток по несуществующим ключам
    const QVariant value(const QStringList& akey, const QVariant &adefault = QVariant()) const;
    const QVariant value(const QString& akey, const QVariant &adefault = QVariant()) const;
    //! Оператор глубокого сравнения словарей
    bool operator==(const JsonDict& src) const;
    bool operator!=(const JsonDict& src) const;
    //! Конвертация в QJsonDocument
    QJsonObject toJsonObj() const;
    //! Заполнение из QJsonDocument
    static JsonDict fromJsonObj(const QJsonObject &json);
    inline bool contains(const QString &key) const {return m_dict.contains(key);}
    bool contains(const QStringList &key) const;
    Dict &data();
    const QString &firstKey() const {return m_dict.firstKey();}
    QVariant &first() {return m_dict.first();}
    const Dict& data() const;
    const Dict& constData() const;
    Dict flatten(const QString &separator) const;
    QStringList keys(const QString &separator) const;
    int remove(const QStringList &akey);
    QVariant take(const QStringList &akey);
    void merge(const JsonDict &src);
    bool isEmpty() const;
    /*!
     * \defgroup JsonDictIterator Iterator
     * \ingroup JsonDict
     * \brief Глубокий итератор.
     *
     *
     * Если функция begin() вызывается на const версию словаря, то создается const_iterator,
     * позволяющий прочитать все значения и получить ключи с доменами, но не дает изменять
     * содержимое
     * @{
     */
    //! Глубокий итератор,
    /// Возвращает пару <поле - значение> (getFieldPair() / field() / value()) или полный ключ (getFullKey()) на любом уровне вложенности,
    /// а также ведет учет домена (getCurrentDomain()), в котором находится эта пара. Позволяет записывать нвоые значения в поля
    class RADAPTER_SHARED_SRC iterator{
        friend class Formatters::JsonDict;
    public:
        //! Эта функция вызывается при "спуске" во время итерации
        /// Переопределите, чтобы добавить обработку спуска на нижний домен
        inline bool operator==(const iterator &src) { return m_iter == src.m_iter; }
        inline bool operator!=(const JsonDict::iterator &src) { return m_iter != src.m_iter; }
        inline iterator& operator*() { return *this; }
        iterator& operator++();
        iterator operator++(int);
        QStringList getCurrentDomain() const;
        QStringList getFullKey() const;
        const QString& field() const;
        QVariant& value() const;
    private:
        iterator(Dict::iterator iter, bool isEmpty);
        Dict::iterator m_iter;
        Dict::iterator m_currentEnd;
        typedef QPair<Dict::iterator,Dict::iterator> iters;
        QStack<iters> m_traverseHistory;
        bool m_firstRun;
        bool m_justReturned;
    };
    //! const_iterator также работает со ссылками, но не позволяет их изменять
    class RADAPTER_SHARED_SRC const_iterator{
        friend class Formatters::JsonDict;
    public:
        //! Эта функция вызывается при "спуске" во время итерации
        /// Переопределите, чтобы добавить обработку спуска на нижний домен
        inline bool operator==(const const_iterator &src) { return m_iter == src.m_iter; }
        inline bool operator!=(const JsonDict::const_iterator &src) { return m_iter != src.m_iter; }
        inline const_iterator& operator*() { return *this; }
        const_iterator& operator++();
        const_iterator operator++(int);
        QStringList getCurrentDomain() const;
        QStringList getFullKey() const;
        const QString& field() const;
        const QVariant& value() const;
    private:
        const_iterator(Dict::const_iterator iter, bool isEmpty);
        Dict::const_iterator m_iter;
        Dict::const_iterator m_currentEnd;
        typedef QPair<Dict::const_iterator,Dict::const_iterator> const_iters;
        QStack<const_iters> m_traverseHistory;
        bool m_firstRun;
        bool m_justReturned;
    };
    /*! @} */ //iterator doxy
    JsonDict::iterator begin();
    JsonDict::iterator end();
    JsonDict::const_iterator begin() const;
    JsonDict::const_iterator end() const;
protected:
    Dict m_dict;
private:
    QString processWarn(const QStringList &src, const int& index);
};
/*! @} */ //JsonDict doxy
Q_DECLARE_METATYPE(Formatters::JsonDict)
#endif // JsonDict_H
