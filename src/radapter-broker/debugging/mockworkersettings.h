#ifndef MOCKWORKERSETTINGS_H
#define MOCKWORKERSETTINGS_H

#include "../radapterbrokerglobal.h"
#include "../workerbasesettings.h"

namespace Radapter {
struct RADAPTER_SHARED_SRC MockWorkerSettings;
}

struct Radapter::MockWorkerSettings : WorkerSettings{
    quint32 mockTimerDelay;
    QString jsonFilePath;
};

#endif // MOCKWORKERSETTINGS_H
