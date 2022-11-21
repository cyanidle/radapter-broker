#include "workermsg.h"
#include "brokerlogging.h"
#include "workerbase.h"

using namespace Radapter;

const QString WorkerMsg::UserTypeField = "__user_msg_type_field__";
const QString WorkerMsg::BrokenMsgReasonField = "__broken_msg_reason_field__";
const QString WorkerMsg::ReplyStatusField = "__status__";
QMutex WorkerMsg::m_mutex;
quint64 WorkerMsg::m_currentMsgId = 0u;

WorkerMsg::WorkerMsg() :
    Formatters::JsonDict(),
    receivers(),
    brokerFlags(BrokerFlags::BrokerBadMsg),
    workerFlags(),
    m_sender(),
    m_schema(nullptr),
    m_id()
{
    operator[](BrokenMsgReasonField) = QString("Created Manualy");
}

WorkerMsg::WorkerMsg(const QObject *sender,
                     const Formatters::JsonDict &msgData,
                     const QVector<QString> &newReceivers) :
    Formatters::JsonDict(msgData),
    receivers(newReceivers),
    brokerFlags(BrokerFlags::BrokerNoAction),
    workerFlags(WorkerFlags::WorkerNormal),
    m_sender(sender),
    m_schema(nullptr)
{
}

const WorkerBase* WorkerMsg::sender() const
{
    return qobject_cast<const WorkerBase*>(m_sender);
}

void WorkerMsg::setReceivers(const QStringList &newReceivers)
{
    receivers.clear();
    for (auto &receiver : newReceivers) {
        receivers.append(receiver);
    }
}

QVariant WorkerMsg::receiveWithSchema() const {
    if (hasSchema()) {
        return m_schema->receive(data());
    } else {
        return QVariant();
    }
}

bool WorkerMsg::usesSchema(const JsonSchema* wantedSchema) const {
    return m_schema->inherits(wantedSchema->metaObject()->className());
}

bool WorkerMsg::isFrom(const QObject* wantedSender) const {
    return m_sender->inherits(wantedSender->metaObject()->className());
}


QString WorkerMsg::brokerFlagsRepr() const {
    switch (brokerFlags){
    case WorkerMsg::BrokerBadMsg:
        return "BadMsg";
        break;
    case WorkerMsg::BrokerBroadcastToAll:
        return "BroadcastToAll";
        break;
    case WorkerMsg::BrokerNoAction:
        return "NoAction";
        break;
    case WorkerMsg::BrokerForwardMsg:
        return "ForwardMsg";
        break;
    default:
        return "Unknown";
        break;
    }
}

QString WorkerMsg::workerFlagsRepr() const {
    switch (workerFlags){
    case WorkerMsg::WorkerInternalCommand:
        return "Command";
        break;
    case WorkerMsg::WorkerReply:
        return "Reply";
        break;
    case WorkerMsg::WorkerNormal:
        return "Normal";
        break;
    default:
        return "Unknown";
        break;
    }
}


const QString WorkerMsg::printFlatData() const
{
    QString result;
    auto flat = flatten(":");
    for (auto keyVal = flat.constBegin(); keyVal != flat.constEnd(); ++keyVal) {
        result.append("# '");
        result.append(keyVal.key());
        result.append("': '");
        result.append(keyVal.value().value<QString>() + "';\n");
    }
    return result;
}


const QString WorkerMsg::printReceivers() const
{
    QString result;
    if (!receivers.isEmpty()) {
        for (const auto &receiver : receivers) {
            result.append(receiver);
            result.append(", ");
        }
    } else {
        result = "None";
    }
    return result;
}

const QString WorkerMsg::printFullDebug() const
{
    auto senderWorker = sender();
    QString schemaName = "None";
    if (hasSchema()) {
        schemaName = schema()->metaObject()->className();
    }
    if (!senderWorker) {
        brokerError() << "Msg ID: " << m_id << " --> sender is not a workerBase!";
        return "Sender Error!";
    }
    return QString("\n ### Msg Id: ") + QString::number(id()) + " Debug Info ###\n" +
           "# " + "Sender: " + senderWorker->workerName()  + "\n" +
           "# " + "Targets: " + printReceivers() +"\n" +
           "# " + "BrokerFlags: " + brokerFlagsRepr() + "\n" +
           "# " + "WorkerFlags: " + workerFlagsRepr() + "\n" +
           "# " + "Schema: " + schemaName + "\n" +
           "# " + "Sender Waiting for Num Replies: " + QString::number(senderWorker->msgQueueSize()) + "\n" +
           "# " + "Flat Msg: \n" +
           printFlatData() +
           "### Msg Id: " + QString::number(id()) + " Debug End  ###";
}
