#include "mockworker.h"
#include <QJsonArray>
#include <QJsonObject>
#include "../brokerlogging.h"

using namespace Radapter;

MockWorker::MockWorker(const MockWorkerSettings &settings) :
    WorkerBase(settings),
    m_file(new QFile(settings.jsonFilePath)),
    m_mockTimer(new QTimer()),
    m_currentIndex()
{
    m_mockTimer->setInterval(settings.mockTimerDelay);
    connect(m_mockTimer, &QTimer::timeout, this, &MockWorker::onMock);
    if (!m_file->isOpen()) {
        if (!m_file->open(QIODevice::ReadOnly)) {
            brokerError() << "Error opening Json file: " << m_file->fileName();
            return;
        }
    }
    QTextStream in(m_file);
    QJsonParseError err;
    const QJsonDocument inputDoc = QJsonDocument::fromJson(in.readAll().toUtf8(), &err);
    const QJsonArray jsonArray = inputDoc.array();
    if (err.error != QJsonParseError::NoError) {
        brokerError() << "Error parsing Json in file: " << m_file->fileName();
        brokerError() << "Full reason: " << err.errorString();
    }
    for (const auto &item : jsonArray) {
        const auto &current = item.toObject().toVariantMap();
        if(!current.isEmpty()) {
            m_jsons.append(current);
        }
    }
    m_file->close();
}

void MockWorker::run()
{
    thread()->start();
    m_mockTimer->start();
}


const Formatters::JsonDict &MockWorker::getNext()
{
    if (m_jsons.isEmpty()) {
        brokerError() << "MockWorker --> Empty Jsons List";
        throw std::runtime_error("MockWorker --> Empty Jsons List");
    }
    if (m_currentIndex >= m_jsons.size()) {
        m_currentIndex = 0;
    }
    return m_jsons.at(m_currentIndex++);
}

void MockWorker::onMock()
{
    if (m_jsons.isEmpty()) {
        return;
    }
    auto currentMsg = getNext();
    auto mockCommands = currentMsg.take({"__mock_commands__"}).toMap();
    if (mockCommands.contains("ignore") && mockCommands.value("ignore").toBool()) {
        onMock();
        return;
    }
    if (mockCommands.contains("once") && mockCommands.value("once").toBool()) {
        m_jsons.removeAt(m_currentIndex);
    }
    /// \todo Add commands for mock behaviour
    const auto msg = prepareMsg(currentMsg);
    emit sendMsg(msg);
}

