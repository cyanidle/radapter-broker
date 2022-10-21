#ifndef WORKERBASE_H
#define WORKERBASE_H

#include <QMutexLocker>
#include <QMutex>
#include <QQueue>
#include "workermsg.h"
#include "workerproxy.h"
#include "interceptorbase.h"
#include "workerbasestub.h"

struct Radapter::WorkerSettings {
    WorkerSettings() :
        name(),
        thread(),
        consumers(),
        producers(),
        maxMsgsInQueue()
    {}
    WorkerSettings(QString name,
                   QThread* thread = nullptr,
                   QStringList consumers = {},
                   QStringList producers = {},
                   quint16 maxMsgsInQueue = 30) :
        name(name),
        thread(thread),
        consumers(consumers),
        producers(producers),
        maxMsgsInQueue(maxMsgsInQueue)
    {}
    QString name;
    QThread* thread;
    QStringList consumers;
    QStringList producers;
    quint16 maxMsgsInQueue;
};

//! You can override onCommand(cosnt WorkerMsg &) / onReply(cosnt WorkerMsg &) / onMsg(cosnt WorkerMsg &)
class Radapter::WorkerBase : public QObject {
    Q_OBJECT
public:
    friend class Protocol;
    friend class CommandBase;
    enum MsgDirection {
        MsgDefault = 0,
        MsgToProducers,
        MsgToConsumers,
        MsgToAll
    };
    explicit WorkerBase(const WorkerSettings &settings);
    //! Фабричный метод, соединяющий объекты в цепь вплоть до прокси, которая является интерфейсом объекта
    /// Interceptor - объект, который находится между прокси и объектом, выполняя некоторую работу над проходящими данными
    WorkerProxy* createProxy(QList<InterceptorBase*> interceptors = QList<InterceptorBase*>(), bool isThreadSafe = true);
    const QString &workerName() const {return m_name;}
    const QStringList &consumers() const {return m_consumers;}
    const QStringList &producers() const {return m_producers;}
    int msgQueueSize() const {return m_asyncQueue.size();}

    virtual void run() = 0;

    virtual ~WorkerBase() = default;
signals:
    void sendMsgExplicit(const Radapter::WorkerMsg &msg, Radapter::WorkerBase::MsgDirection direction);
    void sendMsg(const Radapter::WorkerMsg &msg);
public slots:
    void addConsumers(const QStringList &consumers);
    void addProducers(const QStringList &producers);

    virtual void onReply(const Radapter::WorkerMsg &msg);
    virtual void onCommand(const Radapter::WorkerMsg &msg);
    virtual void onMsg(const Radapter::WorkerMsg &msg);
private slots:
    void onMsgFromBroker(const Radapter::WorkerMsg &msg);
    void sendMsgToImpl(const Radapter::WorkerMsg &msg, Radapter::WorkerBase::MsgDirection direction);
protected:
    WorkerMsg dequeueMsg(quint64 id);
    quint64 enqueueMsg(const WorkerMsg &msg);
    WorkerMsg prepareCommand(const WorkerMsg &msg = WorkerMsg(), MsgDirection direction = MsgDefault) const;
    WorkerMsg prepareMsg(const Formatters::JsonDict &msg = Formatters::JsonDict(), MsgDirection direction = MsgDefault) const;
    WorkerMsg prepareReply(const WorkerMsg &msg = WorkerMsg()) const;
    WorkerMsg prepareMsgBroken(const QString &reason);
private:
    void formatMsgDirection(WorkerMsg *msg, MsgDirection direction) const;

    static void onCreation(const QString &name);
    static quint64 newMsgId() {QMutexLocker locker(&m_mutex); return ++m_currentMsgId;}

    QHash<quint64, WorkerMsg> m_asyncQueue;
    QHash<quint64, quint32> m_asyncQueueCounts;
    quint16 m_maxInAsync;
    QThread* m_thread;
    QString m_name;
    QStringList m_consumers;
    QStringList m_producers;
    WorkerMsg m_baseMsg;
    WorkerProxy* m_proxy;
    bool m_isThreadSafe;

    static quint64 m_currentMsgId;
    static QMutex m_mutex;
    static QStringList m_wereCreated;
    static QList<InterceptorBase*> m_usedInterceptors;
};

#endif //WORKERBASE_H
