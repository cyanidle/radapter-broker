#ifndef CORELOGGING_H
#define CORELOGGING_H

#include <QLoggingCategory>
#include <QDebug>
#include "radapterbrokerglobal.h"

RADAPTER_SHARED_SRC Q_DECLARE_LOGGING_CATEGORY(BrokerLogging);

#define brokerInfo() qCInfo(BrokerLogging)
#define brokerWarn() qCWarning(BrokerLogging)
#define brokerError() qCCritical(BrokerLogging)

#endif // CORELOGGING_H
