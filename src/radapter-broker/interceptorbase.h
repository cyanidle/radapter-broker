#ifndef INTERCEPTORBASE_H
#define INTERCEPTORBASE_H

#include "workermsg.h"
#include "radapterbrokerglobal.h"

namespace Radapter {
class RADAPTER_SHARED_SRC InterceptorBase;
}

//! Класс, перехватывающий трафик
class Radapter::InterceptorBase : public QObject
{
    Q_OBJECT
public:
    explicit InterceptorBase();
signals:
    void msgToWorker(const Radapter::WorkerMsg &msg);
    void msgToBroker(const Radapter::WorkerMsg &msg);
public slots:
    virtual void onMsgFromWorker(const Radapter::WorkerMsg &msg);
    virtual void onMsgFromBroker(const Radapter::WorkerMsg &msg);
public slots:

private:
};

#endif //INTERCEPTORBASE_H
