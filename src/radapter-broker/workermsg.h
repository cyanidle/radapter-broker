#ifndef WORKERMSG_H
#define WORKERMSG_H

#include "JsonFormatters"
#include <QObject>
#include <QDateTime>
#include "radapterbrokerglobal.h"
#include "workerbasestub.h"
#include "jsonschema.h"
#include <QUuid>

namespace Radapter {
class RADAPTER_SHARED_SRC WorkerMsg;
}

class Radapter::WorkerMsg : public Formatters::JsonDict {
public:
    static const QString UserTypeField;
    static const QString BrokenMsgReasonField;
    static const QString ReplyStatusField;
    explicit WorkerMsg();
    enum BrokerFlags {
        BrokerNoAction = 0,
        BrokerForwardMsg,
        BrokerBroadcastToAll,
        BrokerBadMsg
    };
    enum WorkerFlags {
        WorkerNormal = 0,
        WorkerInternalCommand,
        WorkerReply
    };
    
    void setData(const Formatters::JsonDict &msgData) {m_dict = msgData.data();}
    void setReceivers(const QStringList &newReceivers);
    bool isFrom(const QObject* wantedSender) const;
    bool usesSchema(const JsonSchema* wantedSchema) const;
    bool hasSchema() const {return m_schema!=nullptr;}
    QString schemaName() const {return hasSchema() ? m_schema->schemaName() : "None" ;}
    bool isBroken() const {return brokerFlags == BrokerBadMsg;}
    QString brokerFlagsRepr() const;
    QString workerFlagsRepr() const;
    const JsonSchema *schema() const {return m_schema;}
    QVariant receiveWithSchema() const;

    template<class T>
    inline bool isFrom() const {
        static_assert(std::is_base_of<WorkerBase, T>(),
                      "Attempt to check sender for non WorkerBase subclass!");
        return qobject_cast<const T*>(m_sender) != nullptr;
    }
    template<class T>
    inline bool usesSchema() const {
        static_assert(std::is_base_of<JsonSchema, T>(),
                      "Attempt to check schema for non JsonSchema subclass!");
        return qobject_cast<const T*>(m_schema) != nullptr;
    }
    template<class T>
    inline const T* schemaAs() const {
        static_assert(std::is_base_of<JsonSchema, T>(),
                      "Attempt to cast schema for non JsonSchema subclass!");
        return qobject_cast<const T*>(m_schema);
    }
    template<class T>
    inline const T* senderAs() const {
        static_assert(std::is_base_of<WorkerBase, T>(),
                      "Attempt to cast sender for non WorkerBase subclass!");
        return qobject_cast<const T*>(m_sender);
    }
    const quint64 &id() const {return m_id;}
    QVector<QString> receivers;
    BrokerFlags brokerFlags;
    WorkerFlags workerFlags;

    const WorkerBase* sender() const;

    const QString printFullDebug() const;
    const QString printFlatData() const;
    const QString printReceivers() const;
    void updateMsgId() {m_id = newMsgId();}
    static const quint64 &lastMsgId() {return m_currentMsgId;}
private:
    static quint64 newMsgId() {QMutexLocker locker(&m_mutex); return m_currentMsgId++;}
    friend class Radapter::WorkerBase;

    static QMutex m_mutex;
    static quint64 m_currentMsgId;
    const QObject* m_sender;
    const JsonSchema *m_schema;
    quint64 m_id;
    explicit WorkerMsg(const QObject *sender,
                       const Formatters::JsonDict &msgData = {},
                       const QVector<QString> &receivers = QVector<QString>());
};

Q_DECLARE_METATYPE(Radapter::WorkerMsg);

#endif //WORKERMSG_H
