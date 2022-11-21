#include "interceptorbase.h"
#include "workerbase.h"

using namespace Radapter;

void InterceptorBase::onMsgFromWorker(const Radapter::WorkerMsg &msg)
{
    emit msgToBroker(msg);
}

void InterceptorBase::onMsgFromBroker(const Radapter::WorkerMsg &msg)
{
    emit msgToWorker(msg);
}

const WorkerBase *InterceptorBase::worker() const
{
    return qobject_cast<const WorkerBase*>(parent());
}

WorkerBase *InterceptorBase::workerNonConst() const
{
    return qobject_cast<WorkerBase*>(parent());
}
