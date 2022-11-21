#ifndef WORKERBASESETTINGS_H
#define WORKERBASESETTINGS_H

#include "radapterbrokerglobal.h"

namespace Radapter {
struct RADAPTER_SHARED_SRC WorkerSettings;
}

struct Radapter::WorkerSettings {
    WorkerSettings() :
        name(),
        thread(),
        consumers(),
        producers(),
        isDebug(false),
        maxMsgsInQueue(30)
    {}
    WorkerSettings(QString name,
                   QThread* thread = nullptr,
                   QStringList consumers = {},
                   QStringList producers = {},
                   bool isDebug = false,
                   quint16 maxMsgsInQueue = 30) :
        name(name),
        thread(thread),
        consumers(consumers),
        producers(producers),
        isDebug(isDebug),
        maxMsgsInQueue(maxMsgsInQueue)
    {}
    bool isValid() const {return !name.isEmpty();}

    QString name;
    QThread* thread;
    QStringList consumers;
    QStringList producers;
    bool isDebug;
    quint16 maxMsgsInQueue;
};

#endif // WORKERBASESETTINGS_H
