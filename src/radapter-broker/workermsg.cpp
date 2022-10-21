#include "workermsg.h"

using namespace Radapter;

const QString WorkerMsg::UserTypeField = "__user_msg_type_field__";
const QString WorkerMsg::BrokenMsgReasonField = "__broken_msg_reason_field__";

WorkerMsg::WorkerMsg() :
    Formatters::JsonDict(),
    sender(),
    receivers(),
    brokerFlags(BrokerFlags::BrokerBadMsg),
    workerFlags(),
    m_id()
{
    operator[](BrokenMsgReasonField) = QString("Created Manualy");
}

WorkerMsg::WorkerMsg(const QString &sender,
                     const Formatters::JsonDict &msgData,
                     const QVector<QString> &newReceivers) :
    Formatters::JsonDict(msgData),
    sender(sender),
    receivers(newReceivers),
    brokerFlags(BrokerFlags::BrokerNoAction),
    workerFlags(WorkerFlags::WorkerNoAction)
{
}

void WorkerMsg::setReceivers(const QStringList &newReceivers)
{
    receivers.clear();
    for (auto &receiver : newReceivers) {
        receivers.append(receiver);
    }
}
