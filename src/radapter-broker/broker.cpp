#include "broker.h"
#include "brokerlogging.h"
#include "workerbase.h"

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
        brokerWarn() << "Broker: Broken Msg; Sender: " << msg.sender()->objectName() << "; Reason: " << msg[WorkerMsg::BrokenMsgReasonField];
        brokerWarn() << msg.printFullDebug();
    } else {
        brokerWarn() << "Broker: Unknown msg broker flags; Sender: " << msg.sender()->objectName();
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

void Broker::connectProducersAndConsumers(bool pedantic)
{
    if (m_wasMassConnectCalled) {
        brokerError() << "Broker: connectProducersAndConsumers() was already called!";
        throw std::runtime_error("Broker: connectProducersAndConsumers() was already called!");
    }
    m_wasMassConnectCalled = true;
    for (auto proxyIter = m_proxies.constBegin(); proxyIter != m_proxies.constEnd(); ++proxyIter) {
        for (auto &consumer : proxyIter.value()->consumers()) {
            connectTwoProxies(proxyIter.key(), consumer, pedantic);
        }
        for (auto &producer : proxyIter.value()->producers()) {
            connectTwoProxies(producer, proxyIter.key(), pedantic);
        }
    }
}

void Broker::connectTwoProxies(const QString &producerName,
                    const QString &consumerName, bool pedantic)
{
    QMutexLocker locker(&m_mutex);
    if (m_connected.contains({producerName, consumerName})) {
        return;
    }
    if (!m_proxies.contains(producerName)) {
        brokerWarn() << "Broker: connectProxies: No proxy (producer) with name: " << producerName;
        brokerWarn() << "^ Wanted by: " << consumerName;
        if (pedantic) {
            throw std::runtime_error("Broker: connectProxies(): missing producer!");
        }
        return;
    }
    if (!m_proxies.contains(consumerName)) {
        brokerWarn() << "Broker: connectProxies: No proxy (consumer) with name: " << consumerName;
        brokerWarn() << "^ Wanted by: " << producerName;
        if (pedantic) {
            throw std::runtime_error("Broker: connectProxies(): missing consumer!");
        }
        return;
    }
    if (m_proxies.value(producerName)->connectionType() != m_proxies.value(consumerName)->connectionType()) {
        brokerError() <<
                "Broker: connectProxies: Proxies have different threadSafety!: Producer : " <<
                producerName << "; Consumer: " << consumerName;
        throw std::runtime_error("Broker: connectProxies: Proxies have different threadSafety!");
        return;
    }
    auto producer = m_proxies.value(producerName);
    auto consumer = m_proxies.value(consumerName);
    brokerInfo() << "\nConnecting:\n == Producer(" << producer->parent() << ") -->\n == Consumer(" << consumer->parent() << ")";
    connect(producer, &WorkerProxy::msgToBroker,
            consumer, &WorkerProxy::onMsgFromBroker,
            producer->connectionType());
    m_connected.append({producerName, consumerName});
    if (!producer->consumers().contains(consumerName)) {
        producer->addConsumers({consumerName});
    }
    if (!consumer->producers().contains(producerName)) {
        consumer->addProducers({producerName});
    }
}
