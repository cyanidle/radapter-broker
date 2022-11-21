#include "logginginterceptor.h"
#include <QJsonArray>
#include <QJsonObject>
#include "radapter-broker/brokerlogging.h"
#include "radapter-broker/workerbase.h"
#include <QDateTime>

using namespace Radapter;

QMutex LoggingInterceptor::sm_mutex;

LoggingInterceptor::LoggingInterceptor(const LoggingInterceptorSettings &settings) :
    Radapter::InterceptorBase(),
    m_file(new QFile(settings.filePath, this)),
    m_filePath(settings.filePath),
    m_flushTimer(new QTimer(this)),
    m_messages(),
    m_settings(settings),
    m_error(false)
{
    m_flushTimer->setInterval(settings.flushTimerDelay);
    connect(m_flushTimer, &QTimer::timeout, this, &LoggingInterceptor::onFlush);
    m_flushTimer->start();
}


void LoggingInterceptor::openNew(int *count)
{
    if (count == nullptr) {
        int newCount = 0;
        count = &newCount;
    }
    if (*count > 10) {
        brokerError() << "Log Files open-error limit reached!";
        m_error = true;
        return;
    }
    m_file->close();
    int dotAt = m_filePath.lastIndexOf(".");
    QString newName;
    if (dotAt == -1) {
        newName = m_filePath + "-" + QString::number(*count);
    } else {
        newName = m_filePath.left(dotAt) + "-" + QString::number(*count) + m_filePath.right(dotAt);
    }
    m_file->setFileName(newName);
    if (!m_file->open(QIODevice::ReadWrite)) {
        brokerError() << "Could not open file with name: " << m_file->fileName();
        ++*count;
        openNew(count);
    }
}

void LoggingInterceptor::onFlush()
{
    QMutexLocker locker(&sm_mutex);
    if (m_error) {
        brokerError() << "Logger flush is impossible!";
    }
    if (m_messages.isEmpty()) {
        return;
    }
    if (!m_file->isOpen()) {
        if (!m_file->open(QIODevice::ReadWrite)) {
            brokerError() << "Error opening Json file for read: " << m_file->fileName();
            return;
        }
    }
    if (static_cast<quint64>(m_file->size()) > m_settings.maxFileSizeBytes) {
        openNew();
        if (m_error) {
            return;
        }
    }
    QTextStream inOut(m_file);
    QJsonParseError err;
    const QJsonDocument inDoc = QJsonDocument::fromJson(inOut.readAll().toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        brokerWarn() << "All data in file: (" << m_file->fileName() << ") will be deleted!";
    }
    auto array = inDoc.array();
    const QList<Radapter::WorkerMsg> messages = std::move(m_messages);
    for (const auto &msg : messages) {
        if (!testMsgForLog(msg)) {
            continue;
        }
        auto toInsert = msg;
        auto meta = Formatters::JsonDict();
//        meta["timestamp"] = QDateTime::currentDateTimeUtc().toString();
        meta["id"] = msg.id();
        meta["sender"] = msg.sender()->workerName();
        meta["targets"] = msg.printReceivers();
        meta["worker_flags"] = msg.workerFlagsRepr();
        meta["broker_flags"] = msg.brokerFlagsRepr();
        meta["schema"] = msg.brokerFlagsRepr();
        toInsert["__logging_meta__"] = meta.data();
        array.append(toInsert.toJsonObj());
    }
    m_file->resize(0);
    inOut << QJsonDocument(array).toJson(m_settings.format);
    m_file->close();
}


bool LoggingInterceptor::testMsgForLog(const Radapter::WorkerMsg &msg) {
    if (m_settings.logFlags.testFlag(LoggingInterceptorSettings::LogAll)) {
        return true;
    }
    switch (msg.workerFlags) {
    case WorkerMsg::WorkerNormal:
        return m_settings.logFlags.testFlag(
            LoggingInterceptorSettings::LogNormal);
    case WorkerMsg::WorkerReply:
        return m_settings.logFlags.testFlag(
            LoggingInterceptorSettings::LogReply);
    case WorkerMsg::WorkerInternalCommand:
        return m_settings.logFlags.testFlag(
            LoggingInterceptorSettings::LogCommand);
    default:
        return true;
    }
}

void LoggingInterceptor::onMsgFromWorker(const Radapter::WorkerMsg &msg)
{
    m_messages.append(msg);
    emit msgToBroker(msg);
}
