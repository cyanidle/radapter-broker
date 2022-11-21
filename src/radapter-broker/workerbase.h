#ifndef WORKERBASE_H
#define WORKERBASE_H

#include <QMutexLocker>
#include <QMutex>
#include <QHash>
#include <QThread>
#include "workermsg.h"
#include "workerproxy.h"
#include "interceptorbase.h"
#include "workerbasestub.h"
#include "workerbasesettings.h"

//! You can override onCommand(cosnt WorkerMsg &) / onReply(cosnt WorkerMsg &) / onMsg(cosnt WorkerMsg &)
class Radapter::WorkerBase : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isDebug MEMBER m_isDebug)
public:

    bool debugMode() const {return m_isDebug;}

    enum MsgDirection {
        DirectionDefault = 0,
        DirectionToProducers,
        DirectionToConsumers,
        DirectionToAll
    };
    explicit WorkerBase(const WorkerSettings &settings);
    //! Фабричный метод, соединяющий объекты в цепь вплоть до прокси, которая является интерфейсом объекта
    /// Interceptor - объект, который находится между прокси и объектом, выполняя некоторую работу над проходящими данными
    WorkerProxy* createProxy(const QList<InterceptorBase *> &interceptors = QList<InterceptorBase*>(), bool isThreadSafe = true);
    const QString workerName() const {return objectName();}
    const QStringList &consumers() const {return m_consumers;}
    const QStringList &producers() const {return m_producers;}
    int msgQueueSize() const {return m_asyncQueue.size();}

    virtual void run() = 0;
    virtual ~WorkerBase() = default;
signals:
    void sendMsgWithDirection(const Radapter::WorkerMsg &msg, Radapter::WorkerBase::MsgDirection direction);
    void sendMsg(const Radapter::WorkerMsg &msg);
public slots:
    void addConsumers(const QStringList &consumers);
    void addProducers(const QStringList &producers);

    virtual void onReply(const Radapter::WorkerMsg &msg);
    virtual void onCommand(const Radapter::WorkerMsg &msg);
    virtual void onMsg(const Radapter::WorkerMsg &msg);
    virtual void onSendMsg(const Radapter::WorkerMsg &msg) {Q_UNUSED(msg);}
private slots:
    void onSendMsgPriv(const Radapter::WorkerMsg &msg);
    void onMsgFromBroker(const Radapter::WorkerMsg &msg);
    void sendMsgToImpl(const Radapter::WorkerMsg &msg, Radapter::WorkerBase::MsgDirection direction);
protected:
    WorkerMsg dequeueMsg(quint64 id);
    quint64 enqueueMsg(const WorkerMsg &msg);
    template <class Schema>
    WorkerMsg prepareCommand(const QVariant &source, MsgDirection direction = DirectionDefault) const
    {
        static_assert(std::is_base_of<JsonSchema, Schema>(),
                      "Attempt to use schema which is not JsonSchema subclass!");
        auto msg = prepareCommandImpl(Schema::instance()->prepareMsg(source), direction);
        msg.m_schema = Schema::instance();
        return msg;
    }
    WorkerMsg prepareCommand(const JsonSchema *schema, const QVariant &source, MsgDirection direction = DirectionDefault) const;
    WorkerMsg prepareMsg(const Formatters::JsonDict &msg = Formatters::JsonDict(), MsgDirection direction = DirectionDefault) const;
    WorkerMsg prepareReply(const WorkerMsg &msg, const QVariant &status = "ok") const;
    WorkerMsg prepareMsgBroken(const QString &reason);
private:

    WorkerMsg prepareCommandImpl(const Formatters::JsonDict &msg = Formatters::JsonDict(), MsgDirection direction = DirectionDefault) const;
    void formatMsgDirection(WorkerMsg *msg, MsgDirection direction) const;

    static void onCreation(const QString &name);

    QHash<quint64, WorkerMsg> m_asyncQueue;
    QHash<quint64, quint32> m_asyncQueueCounts;
    quint16 m_maxInAsync;
    QThread* m_thread;
    QStringList m_consumers;
    QStringList m_producers;
    WorkerMsg m_baseMsg;
    WorkerProxy* m_proxy;
    bool m_isThreadSafe;
    bool m_isDebug;

    static QMutex m_mutex;
    static QStringList m_wereCreated;
    static QList<InterceptorBase*> m_usedInterceptors;
};

#endif //WORKERBASE_H
