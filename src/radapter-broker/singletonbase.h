#ifndef RADAPTER_CORE_SINGLETONBASE_H
#define RADAPTER_CORE_SINGLETONBASE_H

#include "radapterbrokerglobal.h"
#include "workerbase.h"
#include <QMutexLocker>

namespace Radapter {
class RADAPTER_SHARED_SRC SingletonBase;
} // namespace Radapter

class Radapter::SingletonBase : public Radapter::WorkerBase
{
    Q_OBJECT
public:
    virtual void run() = 0;
    virtual int initSettings() = 0;
    virtual int init() = 0;
protected:
    explicit SingletonBase(const WorkerSettings &settings);
private:
    static QMutex m_mutex;
};



#endif // RADAPTER_CORE_SINGLETONBASE_H
