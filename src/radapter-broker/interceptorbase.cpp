#include "interceptorbase.h"

using namespace Radapter;

InterceptorBase::InterceptorBase() :
    QObject()
{
}

void InterceptorBase::onMsgFromWorker(const Radapter::WorkerMsg &msg)
{
    emit msgToBroker(msg);
}

void InterceptorBase::onMsgFromBroker(const Radapter::WorkerMsg &msg)
{
    emit msgToWorker(msg);
}
