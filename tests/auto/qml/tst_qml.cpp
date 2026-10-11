// SPDX-License-Identifier: Apache-2.0
#include <QtQuickTest/quicktest.h>
#include <QAnimationDriver>
#include <QElapsedTimer>
#include <QTimer>

// Simulate an animation clock advancing faster than real time, as can happen
// when a render loop assumes vsync throttling that the graphics driver ignores.
class FastAnimationDriver : public QAnimationDriver
{
public:
    explicit FastAnimationDriver(QObject *parent) : QAnimationDriver(parent)
    {
        m_timer.setTimerType(Qt::PreciseTimer);
        m_timer.setInterval(4);
        connect(&m_timer, &QTimer::timeout, this, &FastAnimationDriver::advance);
        connect(this, &QAnimationDriver::started, this, [this] {
            m_clock.start();
            m_timer.start();
        });
        connect(this, &QAnimationDriver::stopped, &m_timer, &QTimer::stop);
    }

    void advance() override { advanceAnimation(); }
    qint64 elapsed() const override { return m_clock.isValid() ? m_clock.elapsed() * 5 : 0; }

private:
    QElapsedTimer m_clock;
    QTimer m_timer;
};

class QuickTestSetup : public QObject
{
    Q_OBJECT
public slots:
    void applicationAvailable()
    {
        if (qEnvironmentVariableIntValue("QCATUIKIT_TEST_FAST_ANIMATION_DRIVER"))
        {
            m_driver = new FastAnimationDriver(this);
            m_driver->install();
        }
    }

    void cleanupTestCase()
    {
        delete m_driver;
        m_driver = nullptr;
    }

private:
    FastAnimationDriver *m_driver = nullptr;
};

QUICK_TEST_MAIN_WITH_SETUP(qcatuikit, QuickTestSetup)
#include "tst_qml.moc"
