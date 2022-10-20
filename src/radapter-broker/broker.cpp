#include "broker.h"
#include "brokerlogging.h"

using namespace Radapter;

QMutex Broker::m_mutex;
Broker* Broker::m_instance = nullptr;

Broker::Broker() :
    QObject(),
    m_proxies(),
    m_connected(),
    m_wasMassConnectCalled(false)
{
}

// Doesnt need mutex, bc is always connected via Qt::QueuedConnection
void Broker::onMsgFromWorker(const Radapter::WorkerMsg &msg)
{
    if (msg.brokerFlags == WorkerMsg::BrokerNoAction) {
        return;
    }else if (msg.brokerFlags == WorkerMsg::BrokerBroadcastToAll) {
        emit broadcastToAll(msg);
    } else if (msg.brokerFlags == WorkerMsg::BrokerForwardMsg) {
        // Proxies should filter msg with this flag;
        emit broadcastToAll(msg);
    } else if (msg.brokerFlags == WorkerMsg::BrokerBadMsg) {
        brokerWarn() << "Broker: Broken Msg; Sender: " << msg.sender << "; Reason: " << msg[WorkerMsg::BrokenMsgReasonField];
    } else {
        brokerWarn() << "Broker: Unknown msg broker flags; Sender: " << msg.sender;
    }
}

void Broker::registerProxy(WorkerProxy* proxy)
{
    QMutexLocker locker(&m_mutex);
    connect(this, &Broker::broadcastToAll,
            proxy, &WorkerProxy::onMsgFromBroker, Qt::ConnectionType::QueuedConnection);
    connect(proxy, &WorkerProxy::msgToBroker,
            this, &Broker::onMsgFromWorker, Qt::ConnectionType::QueuedConnection);
    if (m_proxies.contains(proxy->proxyName())) {
        brokerError() << "Broker: Proxy with duplicate name: " << proxy->proxyName();
        throw std::runtime_error(std::string("Broker: Proxy with duplicate name: ") +
                                 std::string(proxy->proxyName().toStdString()));
    } else {
        brokerInfo() << "Broker: Registering proxy with name: " << proxy->proxyName();
        m_proxies.insert(proxy->proxyName(), proxy);
    }
}

void Broker::connectProducersAndConsumers()
{
    if (m_wasMassConnectCalled) {
        brokerError() << "Broker: connectProducersAndConsumers() was already called!";
        throw std::runtime_error("Broker: connectProducersAndConsumers() was already called!");
    }
    m_wasMassConnectCalled = true;
    for (auto proxyIter = m_proxies.constBegin(); proxyIter != m_proxies.constEnd(); ++proxyIter) {
        for (auto &consumer : proxyIter.value()->consumers()) {
            connectTwoProxies(proxyIter.key(), consumer);
        }
        for (auto &producer : proxyIter.value()->producers()) {
            connectTwoProxies(producer, proxyIter.key());
        }
    }
}

void Broker::connectTwoProxies(const QString &producerName,
                    const QString &consumerName)
{
    QMutexLocker locker(&m_mutex);
    if (m_connected.contains({producerName, consumerName})) {
        return;
    }
    if (!m_proxies.contains(producerName)) {
        brokerWarn() << "Broker: connectProxies: No proxy (producer) with name: " << producerName;
        brokerWarn() << "^ Wanted by: " << consumerName;
        return;
    }
    if (!m_proxies.contains(consumerName)) {
        brokerWarn() << "Broker: connectProxies: No proxy (consumer) with name: " << consumerName;
        brokerWarn() << "^ Wanted by: " << producerName;
        return;
    }
    if (m_proxies.value(producerName)->connectionType() != m_proxies.value(consumerName)->connectionType()) {
        brokerError() <<
                "Broker: connectProxies: Proxies have different threadSafety!: Producer : " <<
                producerName << "; Consumer: " << consumerName;
        throw std::runtime_error("Broker: connectProxies: Proxies have different threadSafety!");
        return;
    }
    brokerInfo() << "Broker: Connecting: Producer(" << producerName << ") --> Consumer(" << consumerName << ")";
    auto producer = m_proxies.value(producerName);
    auto consumer = m_proxies.value(consumerName);
    connect(producer, &WorkerProxy::msgToBroker,
            consumer, &WorkerProxy::onMsgFromBroker,
            producer->connectionType());
    m_connected.append({producerName, consumerName});
    if (!producer->consumers().contains(consumerName)) {
        producer->addConsumers({consumerName});
    }
}
