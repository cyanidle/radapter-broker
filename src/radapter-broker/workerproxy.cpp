#include "workerproxy.h"
#include "brokerlogging.h"
#include "workerbase.h"

using namespace Radapter;

WorkerProxy::WorkerProxy(const QString &name,
                         Qt::ConnectionType connectionType) :
    QObject(),
    m_connectionType(connectionType)
{
    setObjectName(name);
}

const QStringList &WorkerProxy::consumers() const
{
    return worker()->consumers();
}
const QStringList &WorkerProxy::producers() const
{
    return worker()->producers();
}

const WorkerBase* WorkerProxy::worker() const
{
    return qobject_cast<const WorkerBase *>(parent());
}

WorkerBase* WorkerProxy::workerNonConst() const
{
    return qobject_cast<WorkerBase *>(parent());
}

void WorkerProxy::onMsgFromWorker(const Radapter::WorkerMsg &msg)
{
    // If proxies are already connected, do not forward with broker
    if (msg.brokerFlags == WorkerMsg::BrokerForwardMsg) {
        QVector<QString> direct;
        QVector<QString> forwarded;
        for (const auto &receiver : msg.receivers) {
            if (consumers().contains(receiver)) {
                direct.append(receiver);
            } else {
                forwarded.append(receiver);
            }
        }
        if (!direct.isEmpty()) {
            auto directMsg = msg;
            directMsg.brokerFlags = WorkerMsg::BrokerNoAction;
            directMsg.receivers.swap(direct);
            emit msgToBroker(directMsg);
        }
        if (!forwarded.isEmpty()) {
            auto forwardedMsg = msg;
            forwardedMsg.receivers.swap(forwarded);
            emit msgToBroker(forwardedMsg);
        }
    } else {
        emit msgToBroker(msg);
    }
}

const QString WorkerProxy::proxyName() const
{
    return worker()->workerName();
}

void WorkerProxy::onMsgFromBroker(const Radapter::WorkerMsg &msg)
{
    if (msg.brokerFlags == WorkerMsg::BrokerForwardMsg) {
        if (msg.receivers.contains(proxyName())) {
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
    workerNonConst()->addProducers(producers);
}

void WorkerProxy::addConsumers(const QStringList &consumers)
{
    workerNonConst()->addConsumers(consumers);
}
