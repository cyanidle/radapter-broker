#ifndef WORKERMSG_H
#define WORKERMSG_H

#include "../json-formatters/formatters/jsondict.h"
#include <QObject>
#include <QDateTime>
#include <QUuid>
#include "radapterbrokerglobal.h"
#include "workerbasestub.h"

namespace Radapter {
class RADAPTER_SHARED_SRC WorkerMsg;
}

class Radapter::WorkerMsg : public Formatters::JsonDict {
public:
    static const QString UserTypeField;
    static const QString BrokenMsgReasonField;
    explicit WorkerMsg();
    friend class Radapter::WorkerBase;
    enum BrokerFlags {
        BrokerNoAction = 0,
        BrokerForwardMsg,
        BrokerBroadcastToAll,
        BrokerBadMsg
    };
    enum WorkerFlags {
        WorkerNoAction = 0,
        WorkerInternalCommand,
        WorkerReply
    };
    enum SenderType {
        TypeUserDefined = -1,
        TypeUnspecified = 0,
        TypeRedisCacheConsumer,
        TypeRedisCacheProducer,
        TypeRedisKeyEventsConsumer,
        TypeRedisStreamProducer,
        TypeRedisStreamConsumer,
        TypeModbusConnector,
        TypeWebsocketClient,
        TypeWebsockerServerConnector,
        TypeSqlArchiveProducer
    };
    const QDateTime &getTimestamp() const;
    void setData(const Formatters::JsonDict &msgData) {m_dict = msgData.data();}
    void setReceivers(const QStringList &newReceivers);
    const quint64 &id() const {return m_id;}

    QString sender;
    SenderType senderType;
    QVector<QString> receivers;
    BrokerFlags brokerFlags;
    WorkerFlags workerFlags;

private:
    quint64 m_id;
    explicit WorkerMsg(const QString &sender,
                       const Formatters::JsonDict &msgData = {},
                       const QVector<QString> &receivers = QVector<QString>());
};

Q_DECLARE_METATYPE(Radapter::WorkerMsg);

#endif //WORKERMSG_H
