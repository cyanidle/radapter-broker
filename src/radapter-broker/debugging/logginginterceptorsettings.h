#ifndef LOGGINGINTERCEPTORSETTINGS_H
#define LOGGINGINTERCEPTORSETTINGS_H

#include "../radapterbrokerglobal.h"

namespace Radapter {
struct RADAPTER_SHARED_SRC LoggingInterceptorSettings;
}

struct Radapter::LoggingInterceptorSettings{
    enum LogMsgTypesFlag {
        LogAll = 0x0000,
        LogNormal = 0x0001,
        LogReply = 0x0002,
        LogCommand = 0x0004
    };
    Q_DECLARE_FLAGS(LogMsg, LogMsgTypesFlag)
    QString filePath;
    quint32 flushTimerDelay = 1000;
    QJsonDocument::JsonFormat format = QJsonDocument::Indented;
    quint64 maxFileSizeBytes = 100000000; //<100mb
    quint16 maxFiles = 10;
    LogMsg logFlags = LogAll;
};

#endif // LOGGINGINTERCEPTORSETTINGS_H
