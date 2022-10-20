#ifndef WORKERPROXY_H
#define WORKERPROXY_H

#include "workermsg.h"
#include "radapterbrokerglobal.h"
#include "workerbasestub.h"

namespace Radapter {
class RADAPTER_SHARED_SRC WorkerProxy;
}

class Radapter::WorkerProxy : public QObject
{
    Q_OBJECT
public:
    friend class Radapter::WorkerBase;
    const QString &proxyName() const {return m_name;}
    const QStringList &consumers() const {return m_consumers;}
    const QStringList &producers() const {return m_producers;}
    Qt::ConnectionType connectionType() const {return m_connectionType;}
    void addProducers(const QStringList &producer);
    void addConsumers(const QStringList &producer);
signals:
    void msgToWorker(const Radapter::WorkerMsg &msg);
    void msgToBroker(const Radapter::WorkerMsg &msg);
public slots:
    void onMsgFromWorker(const Radapter::WorkerMsg &msg);
    void onMsgFromBroker(const Radapter::WorkerMsg &msg);
private:
    explicit WorkerProxy(const QString &name,
                         const QStringList &consumers = QStringList(),
                         const QStringList &producers = QStringList(),
                         Qt::ConnectionType connectionType = Qt::QueuedConnection,
                         QObject* parent = nullptr);
    QString m_name;
    QStringList m_consumers;
    QStringList m_producers;
    Qt::ConnectionType m_connectionType;
};

#endif //WORKERPROXY_H
