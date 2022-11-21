#include "workerbase.h"
#include "brokerlogging.h"
#include <QThread>

using namespace Radapter;

QMutex WorkerBase::m_mutex;
QStringList WorkerBase::m_wereCreated = QStringList();
QList<InterceptorBase*> WorkerBase::m_usedInterceptors = {};

void WorkerBase::onCreation(const QString &name)
{
    QMutexLocker locker(&m_mutex);
    if (m_wereCreated.contains(name)) {
        brokerError() << "WorkerBase: Attempt to create worker with duplicate name: " << name;
    } else {
        m_wereCreated.append(name);
    }
}

WorkerBase::WorkerBase(const WorkerSettings &settings) :
    QObject(),
    m_asyncQueue(),
    m_asyncQueueCounts(),
    m_maxInAsync(settings.maxMsgsInQueue),
    m_thread(settings.thread),
    m_consumers(settings.consumers),
    m_producers(settings.producers),
    m_baseMsg(this),
    m_proxy(nullptr),
    m_isThreadSafe(true),
    m_isDebug(settings.isDebug)
{
    setObjectName(settings.name);
    if (m_isDebug) {
        brokerWarn()<< "=== Worker (" << workerName() << "): Running in Debug Mode! ===";
    }
    m_baseMsg.brokerFlags = WorkerMsg::BrokerNoAction;
    m_baseMsg.workerFlags = WorkerMsg::WorkerNormal;
    if (m_thread) {
        m_isThreadSafe = false;
    } else {
        m_thread = QThread::currentThread();
    }
    moveToThread(m_thread);
    onCreation(workerName());
    for (auto &consumer : m_consumers) {
        m_baseMsg.receivers.append(consumer);
    }
    connect(this, &WorkerBase::sendMsgWithDirection, this, &WorkerBase::sendMsgToImpl);
    connect(this, &WorkerBase::sendMsg, this, &WorkerBase::onSendMsgPriv);
    connect(this, &WorkerBase::sendMsg, this, &WorkerBase::onSendMsg);
}


void WorkerBase::addConsumers(const QStringList &consumers)
{
    m_consumers.append(consumers);
    for (auto &consumer : consumers) {
        m_baseMsg.receivers.append(consumer);
    }
}

void WorkerBase::addProducers(const QStringList &producers)
{
    m_producers.append(producers);
}

WorkerMsg WorkerBase::prepareMsgBroken(const QString &reason)
{
    auto msg = m_baseMsg;
    msg.updateMsgId();
    msg.brokerFlags = WorkerMsg::BrokerBadMsg;
    msg[WorkerMsg::BrokenMsgReasonField] = reason.isEmpty()?"Not given":reason;
    return msg;
}

WorkerMsg WorkerBase::dequeueMsg(quint64 id)
{
    if (m_asyncQueue.size() == 0) {
        auto msg = m_baseMsg;
        msg.updateMsgId();
        msg.brokerFlags = WorkerMsg::BrokerBadMsg;
        return prepareMsgBroken("Attempt to reply, while reply Queue is empty");
    }
    if (!m_asyncQueue.contains(id)) {
        return prepareMsgBroken(QString("Attempt to reply to id, which was already taken! Id: %1").arg(id));
    }
    if (!--m_asyncQueueCounts[id]) {
        m_asyncQueueCounts.remove(id);
        return m_asyncQueue.take(id);
    } else {
        return m_asyncQueue.value(id);
    }
}

quint64 WorkerBase::enqueueMsg(const WorkerMsg &msg)
{
    if (m_asyncQueue.size() > m_maxInAsync) {
        brokerWarn() << "Worker (" << workerName() << "): too many Msgs awaiting responce: " << m_asyncQueue.size();
    }
    if(!m_asyncQueueCounts[msg.m_id]++) {
        m_asyncQueue.insert(msg.m_id, msg);
    }
    return msg.m_id;
}

void WorkerBase::formatMsgDirection(WorkerMsg *msg, MsgDirection direction) const
{
    msg->brokerFlags = WorkerMsg::BrokerForwardMsg;
    switch (direction) {
    case DirectionDefault:
        return;
    case DirectionToConsumers:
        msg->receivers.clear();
        for (auto &consumer : m_consumers) {
            msg->receivers.append(consumer);
        }
        return;
    case DirectionToProducers:
        msg->receivers.clear();
        for (auto &producer : m_producers) {
            msg->receivers.append(producer);
        }
        return;
    case DirectionToAll:
        msg->receivers.clear();
        for (auto &consumer : m_consumers) {
            msg->receivers.append(consumer);
        }
        for (auto &producer : m_producers) {
            msg->receivers.append(producer);
        }
        return;
    }
}

void WorkerBase::sendMsgToImpl(const Radapter::WorkerMsg &msg, MsgDirection direction)
{
    auto copy = msg;
    formatMsgDirection(&copy, direction);
    emit sendMsg(copy);
}


WorkerMsg WorkerBase::prepareCommand(const JsonSchema *schema, const QVariant &source, MsgDirection direction) const
{
    auto msg = prepareCommandImpl(schema->prepareMsg(source), direction);
    msg.m_schema = schema;
    return msg;
}

WorkerMsg WorkerBase::prepareCommandImpl(const Formatters::JsonDict &msg, MsgDirection direction) const
{
    auto wrapped = m_baseMsg;
    wrapped.updateMsgId();
    wrapped.setData(msg.data());
    formatMsgDirection(&wrapped, direction);
    wrapped.workerFlags = WorkerMsg::WorkerInternalCommand;
    wrapped.brokerFlags = WorkerMsg::BrokerForwardMsg;
    return wrapped;
}

WorkerMsg WorkerBase::prepareMsg(const Formatters::JsonDict &msg, MsgDirection direction) const
{
    auto wrapped = m_baseMsg;
    wrapped.updateMsgId();
    formatMsgDirection(&wrapped, direction);
    if (msg.isEmpty()) {
        wrapped.brokerFlags = WorkerMsg::BrokerBadMsg;
        return wrapped;
    } else {
        wrapped.setData(msg);
        return wrapped;
    }
}

WorkerMsg WorkerBase::prepareReply(const WorkerMsg &msg, const QVariant &status) const
{
    if (msg.brokerFlags == WorkerMsg::BrokerBadMsg) {
        return msg;
    }
    auto reply = msg;
    reply.receivers.clear();
    reply.m_sender = this;
    if (msg.m_sender) {
        reply.receivers.append(qobject_cast<const WorkerBase*>(msg.m_sender)->workerName());
    }
    reply.workerFlags = WorkerMsg::WorkerReply;
    reply.brokerFlags = WorkerMsg::BrokerForwardMsg;
    reply[WorkerMsg::ReplyStatusField] = status;
    return reply;
}

void WorkerBase::onReply(const Radapter::WorkerMsg &msg)
{
    brokerWarn() << metaObject()->className() << "(" <<
        workerName() << "): received Reply from: " <<
        msg.m_sender->metaObject()->className() <<
        "(" << msg.m_sender->objectName() << "), but not handled!";
}

void WorkerBase::onCommand(const Radapter::WorkerMsg &msg)
{
    brokerWarn() << metaObject()->className() << "(" <<
        workerName() << "): received Command from: " <<
        msg.m_sender->metaObject()->className() <<
        "(" << msg.m_sender->objectName() << "), but not handled!";
}

void WorkerBase::onMsg(const Radapter::WorkerMsg &msg)
{
    brokerWarn().noquote() << metaObject()->className() << "(" <<
        workerName() << "): received Generic msg from: " <<
        msg.m_sender->metaObject()->className() <<
        "(" << msg.m_sender->objectName() << "), but not handled!";
}

void WorkerBase::onMsgFromBroker(const Radapter::WorkerMsg &msg)
{
    if (m_isDebug) {
        brokerInfo().noquote() << "\n ||| To Worker: " << workerName() <<  "||| " << msg.printFullDebug();
    }
    switch (msg.workerFlags) {
    case WorkerMsg::WorkerNormal: onMsg(msg); break;
    case WorkerMsg::WorkerInternalCommand: onCommand(msg); break;
    case WorkerMsg::WorkerReply: onReply(msg); break;
    default: brokerWarn() << "Unknown Worker Flag in Msg from: " << msg.sender(); break;
    }
}

void WorkerBase::onSendMsgPriv(const Radapter::WorkerMsg &msg)
{
    if (m_isDebug) {
        brokerInfo().noquote() << "\n ||| From Worker: " << workerName() <<  "||| " << msg.printFullDebug();
    }
}

WorkerProxy* WorkerBase::createProxy(const QList<InterceptorBase*> &interceptors, bool isThreadSafe)
{
    QMutexLocker locker(&m_mutex);
    Qt::ConnectionType connectionType;
    if (isThreadSafe) {
        connectionType = Qt::QueuedConnection;
    } else {
        if (!m_isThreadSafe) {
            brokerError() << "WorkerBase: Attempt to create non-threadsafe proxy, while being in another QThread!";
            throw std::runtime_error("WorkerBase: Attempt to create non-threadsafe proxy, while being in another QThread!");
        }
        connectionType = Qt::AutoConnection;
    }
    if (m_proxy == nullptr) {
        m_proxy = new WorkerProxy(workerName(), connectionType);
        m_proxy->moveToThread(m_thread);
        m_proxy->setParent(this);
        auto filtered = QList<InterceptorBase*>();
        for (auto &interceptor : interceptors) {
            if (!filtered.contains(interceptor) || !m_usedInterceptors.contains(interceptor)) {
                interceptor->setParent(nullptr);
                interceptor->moveToThread(m_thread);
                interceptor->setParent(this);
                filtered.append(interceptor);
                m_usedInterceptors.append(interceptor);
            } else {
                brokerError() << "=======================================================================================";
                brokerError() << "WorkerBase::CreateProxy(): Interceptors list contains copies or already used ones!";
                brokerError() << "WorkerBase::CreateProxy(): This will result in msg loops! Aborting!";
                brokerError() << "=======================================================================================";
                throw std::runtime_error("Interceptors list contains copies! Will result in msg loops!");
                return m_proxy;
            }
        }
        if (filtered.isEmpty()) {
            // Подключение К брокеру (К прокси)
            connect(this, &WorkerBase::sendMsg,
                    m_proxy, &WorkerProxy::onMsgFromWorker,
                    connectionType);
            // Подключение ОТ брокера (ОТ прокси)
            connect(m_proxy, &WorkerProxy::msgToWorker,
                    this, &WorkerBase::onMsgFromBroker,
                    connectionType);
        } else {
            // Подключение К перехватчику (первому/ближнему к воркеру)
            connect(this, &WorkerBase::sendMsg,
                    filtered[0], &InterceptorBase::onMsgFromWorker,
                    Qt::AutoConnection);
            // Подключение ОТ перехватчика (первого)
            connect(filtered[0], &InterceptorBase::msgToWorker,
                    this, &WorkerBase::onMsgFromBroker,
                    Qt::AutoConnection);
            // Если перехватчиков более одного, то соединяем их друг с другом
            for (int i = 0; i < filtered.length() - 1; ++i) {
                connect(filtered[i], &InterceptorBase::msgToBroker,
                        filtered[i + 1], &InterceptorBase::onMsgFromWorker,
                        Qt::AutoConnection);
                connect(filtered[i + 1], &InterceptorBase::msgToWorker,
                        filtered[i], &InterceptorBase::onMsgFromBroker,
                        Qt::AutoConnection);
            }
            // Подключение (последний/дальний от воркера) перехватчик --> прокси
            connect(filtered.last(), &InterceptorBase::msgToBroker,
                    m_proxy, &WorkerProxy::onMsgFromWorker,
                    connectionType);
            // Подключение прокси --> (последний) перехватчик
            connect(m_proxy, &WorkerProxy::msgToWorker,
                    filtered.last(), &InterceptorBase::onMsgFromBroker,
                    connectionType);
        }
    }
    return m_proxy;
}
