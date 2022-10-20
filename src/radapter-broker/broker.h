#ifndef BROKER_H
#define BROKER_H

#include <QMutex>
#include <QMutexLocker>
#include "workerproxy.h"
#include "radapterbrokerglobal.h"

namespace Radapter {
class RADAPTER_SHARED_SRC Broker;
}

class Radapter::Broker : public QObject
{
    Q_OBJECT
public:
    static Broker* instance() {
        QMutexLocker locker(&m_mutex);
        if (!m_instance) {
            m_instance = new Broker();
        }
        return m_instance;
    }
    void registerProxy(WorkerProxy* proxy);
    void connectProducersAndConsumers();
    void connectTwoProxies(const QString &producerName,
                        const QString &consumerName);
    ~Broker() override {
        m_instance = nullptr;
    }
signals:
    void broadcastToAll(const Radapter::WorkerMsg &msg);
private slots:
    void onMsgFromWorker(const Radapter::WorkerMsg &msg);
private:
    explicit Broker();
    QMap<QString, WorkerProxy*> m_proxies;
    QList<QPair<QString, QString>> m_connected;
    bool m_wasMassConnectCalled;
    static Broker* m_instance;
    static QMutex m_mutex;
};

#endif //BROKER_H
