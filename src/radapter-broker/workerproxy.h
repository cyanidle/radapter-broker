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
    const QString proxyName() const;
    const QStringList &consumers() const;
    const QStringList &producers() const;
    Qt::ConnectionType connectionType() const {return m_connectionType;}
    void addProducers(const QStringList &producer);
    void addConsumers(const QStringList &producer);
    const WorkerBase *worker() const;
signals:
    void msgToWorker(const Radapter::WorkerMsg &msg);
    void msgToBroker(const Radapter::WorkerMsg &msg);
public slots:
    void onMsgFromWorker(const Radapter::WorkerMsg &msg);
    void onMsgFromBroker(const Radapter::WorkerMsg &msg);
protected:
    WorkerBase *workerNonConst() const;
private:
    friend class Radapter::WorkerBase;

    explicit WorkerProxy(const QString &name,
                         Qt::ConnectionType connectionType = Qt::QueuedConnection);
    Qt::ConnectionType m_connectionType;
};

#endif //WORKERPROXY_H
