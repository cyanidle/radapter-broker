#ifndef LIST_H
#define LIST_H

#ifndef RADAPTER_SHARED_SRC
#define RADAPTER_SHARED_SRC
#endif

#include <QVariantList>
#include "dict.h"
#include <initializer_list>

namespace Formatters{
    class RADAPTER_SHARED_SRC List;
}
/*!
 * \defgroup List List
 * \ingroup Core
 * \brief Класс-наследник QVariantList.
 *  @{
 */
class Formatters::List : public QList<QVariant>
{
public:
    explicit List();
    List(const QVariantList &src);
    List(const QStringList &src);
    explicit List(const std::initializer_list<Formatters::Dict> &jsons);
    explicit List(const std::initializer_list<QVariant> &values);
    /*!
     *  \defgroup ListAdded Добавленные методы
     *  \ingroup List
     *  @{
    */
    //! Данный метод объединяет словари внутри себя.
    /// Стоит учесть, что значения под повторяющимися ключами перезаписываются
    /// значениями из **последнего** ключа.
    Dict join() const;
    QStringList toQStringList() const;
    /*! @} */
private:
};
/*! @} */
Q_DECLARE_METATYPE(Formatters::List);

#endif // LIST_H
