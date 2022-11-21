#ifndef MOCK_WORKER_H
#define MOCK_WORKER_H

#include <QTimer>
#include <QFile>
#include "mockworkersettings.h"
#include "../workerbase.h"


namespace Radapter {
class RADAPTER_SHARED_SRC MockWorker;
}

class Radapter::MockWorker : public Radapter::WorkerBase
{
    Q_OBJECT
public:
    explicit MockWorker(const MockWorkerSettings &settings);
    void run() override;
private slots:
    void onMock();
private:
    const Formatters::JsonDict &getNext();

    QFile* m_file;
    QTimer* m_mockTimer;
    QList<Formatters::JsonDict> m_jsons;
    int m_currentIndex;
};

#endif
