#include "singletonbase.h"
#include "brokerlogging.h"
#include "broker.h"

using namespace Radapter;

QMutex SingletonBase::m_mutex;


SingletonBase::SingletonBase(const WorkerSettings &settings) :
    WorkerBase(settings)
{
}

