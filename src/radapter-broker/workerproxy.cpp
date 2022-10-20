#include "workerproxy.h"

using namespace Radapter;

WorkerProxy::WorkerProxy(const QString &name,
                         const QStringList &consumers,
                         const QStringList &producers,
                         Qt::ConnectionType connectionType,
                         QObject* parent) :
    QObject(parent),
    m_name(name),
    m_consumers(consumers),
    m_producers(producers),
    m_connectionType(connectionType)
{
}

void WorkerProxy::onMsgFromWorker(const Radapter::WorkerMsg &msg)
{
    emit msgToBroker(msg);
}

void WorkerProxy::onMsgFromBroker(const Radapter::WorkerMsg &msg)
{
    if (msg.brokerFlags == WorkerMsg::BrokerForwardMsg) {
        if (msg.receivers.contains(m_name)) {
            emit msgToWorker(msg);
        }
        return;
    } else if (msg.brokerFlags == WorkerMsg::BrokerBadMsg) {
        return;
    }
    emit msgToWorker(msg);
}

void WorkerProxy::addProducers(const QStringList &producers)
{
    m_producers.append(producers);
    parent()->metaObject()->invokeMethod(parent(),
                                         "addProducers",
                                         Qt::DirectConnection,
                                         Q_ARG(QStringList, producers));
}

void WorkerProxy::addConsumers(const QStringList &consumers)
{
    m_consumers.append(consumers);
    parent()->metaObject()->invokeMethod(parent(),
                                         "addConsumers",
                                         Qt::DirectConnection,
                                         Q_ARG(QStringList, consumers));

}
