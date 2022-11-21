#ifndef LOGGING_INTERCEPTOR_H
#define LOGGING_INTERCEPTOR_H

#include <QTimer>
#include <QFile>
#include "logginginterceptorsettings.h"
#include "../workerbasestub.h"
#include "../interceptorbase.h"

namespace Radapter {
class RADAPTER_SHARED_SRC LoggingInterceptor;
}

class Radapter::LoggingInterceptor : public Radapter::InterceptorBase
{
    Q_OBJECT
public:
    explicit LoggingInterceptor(const LoggingInterceptorSettings &settings);
public slots:
    virtual void onMsgFromWorker(const Radapter::WorkerMsg &msg) override;
private slots:
    void onFlush();
private:
    bool testMsgForLog(const Radapter::WorkerMsg &msg);
    void openNew(int *count = nullptr);

    QFile *m_file;
    QString m_filePath;
    QTimer *m_flushTimer;
    QList<Radapter::WorkerMsg> m_messages;
    LoggingInterceptorSettings m_settings;

    bool m_error;
    static QMutex sm_mutex;
};
#endif
