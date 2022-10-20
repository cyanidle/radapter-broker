#ifndef FACTORYBASE_H
#define FACTORYBASE_H

#include <QObject>
#include "radapterbrokerglobal.h"
#include "workerbase.h"

namespace Radapter {
class RADAPTER_SHARED_SRC FactoryBase;
typedef QList<WorkerBase*> WorkersList;
}

class Radapter::FactoryBase : public QObject
{
    Q_OBJECT
public:
    explicit FactoryBase(QObject *parent = nullptr);
    virtual void run() = 0;
    virtual int initSettings() = 0;
    virtual int initWorkers() = 0;
signals:

};

#endif // FACTORYBASE_H
