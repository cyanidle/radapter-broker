#ifndef INTERCEPTORBASE_H
#define INTERCEPTORBASE_H

#include "workermsg.h"

namespace Radapter {
class RADAPTER_SHARED_SRC InterceptorBase;
typedef QList<InterceptorBase*> Interceptors;
}

//! Класс, перехватывающий трафик
class Radapter::InterceptorBase : public QObject
{
    Q_OBJECT
public:
    explicit InterceptorBase() = default;
    const WorkerBase* worker() const;
signals:
    void msgToWorker(const Radapter::WorkerMsg &msg);
    void msgToBroker(const Radapter::WorkerMsg &msg);

public slots:
    virtual void onMsgFromWorker(const Radapter::WorkerMsg &msg);
    virtual void onMsgFromBroker(const Radapter::WorkerMsg &msg);
protected:
    WorkerBase* workerNonConst() const;
public slots:

private:
};

#endif //INTERCEPTORBASE_H
