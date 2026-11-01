// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/CommandLine.h"
#include "core/Config.h"
#include "core/Logging.h"
#include "wm/WindowModel.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest/QtTest>

namespace Tape {

class TestConfigAndLogging : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testCommandLineParsing()
    {
        const QStringList args = {
            QStringLiteral("tape"),
            QStringLiteral("--nested"),
            QStringLiteral("--outputs"),
            QStringLiteral("2"),
            QStringLiteral("--output-size"),
            QStringLiteral("1920x1080@1.5"),
            QStringLiteral("--debug-categories"),
            QStringLiteral("wm,drm"),
            QStringLiteral("--socket"),
            QStringLiteral("tape-unit-0"),
        };

        const CommandLineOptions opts = CommandLine::parse(args);
        QVERIFY(opts.parseOk);
        QVERIFY(opts.nested);
        QVERIFY(!opts.headless);
        QCOMPARE(opts.outputCount, 2);
        QCOMPARE(opts.outputSpec.size, QSize(1920, 1080));
        QCOMPARE(opts.outputSpec.scale, 1.5);
        QVERIFY(opts.debug);
        QCOMPARE(opts.debugCategories, (QStringList { QStringLiteral("wm"), QStringLiteral("drm") }));
        QCOMPARE(opts.socketName, QStringLiteral("tape-unit-0"));

        // Invalid output size should fail cleanly
        const CommandLineOptions badOpts = CommandLine::parse({
            QStringLiteral("tape"),
            QStringLiteral("--output-size"),
            QStringLiteral("invalid"),
        });
        QVERIFY(!badOpts.parseOk);
    }

    void testLoggingAndRuntimeToggle()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        LogManager* logMgr = LogManager::instance();
        logMgr->shutdown();
        logMgr->initialize(tempDir.path());

        QVERIFY(!logMgr->isDebugEnabled());
        QVERIFY(!lcWm().isDebugEnabled());
        QVERIFY(lcCore().isInfoEnabled());

        QSignalSpy spy(logMgr, &LogManager::debugEnabledChanged);

        // Enable debug only for tape.wm
        logMgr->setRules(true, { QStringLiteral("wm") });
        QVERIFY(logMgr->isDebugEnabled());
        QCOMPARE(spy.count(), 1);
        QVERIFY(lcWm().isDebugEnabled());
        QVERIFY(!lcDrm().isDebugEnabled());

        qCDebug(lcWm, "Test debug message from wm category");

        // Toggle off
        logMgr->toggleDebug();
        QVERIFY(!logMgr->isDebugEnabled());
        QVERIFY(!lcWm().isDebugEnabled());

        // Verify log file was created and contains our message
        QFile logFile(logMgr->logFilePath());
        QVERIFY(logFile.exists());
        QVERIFY(logFile.open(QIODevice::ReadOnly | QIODevice::Text));
        const QString content = QString::fromUtf8(logFile.readAll());
        QVERIFY(content.contains(QStringLiteral("Test debug message from wm category")));

        logMgr->shutdown();
    }

    void testConfigDefaultsAndWrite()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const QString cfgPath = tempDir.filePath(QStringLiteral("taperc"));

        Config cfg(cfgPath);
        QCOMPARE(cfg.focusPolicy(), QStringLiteral("Click"));
        QCOMPARE(cfg.decorationButtonStyle(), QStringLiteral("Breeze"));
        QCOMPARE(cfg.topBarHeight(), 28);
        QCOMPARE(cfg.dockIconSize(), 48);
        QVERIFY(cfg.wobblyEnabled());
        QVERIFY(cfg.magicLampEnabled());
        QVERIFY(cfg.blurEnabled());
        QCOMPARE(cfg.blurStrength(), 6);
        QVERIFY(cfg.inputMethodEnabled());
        QCOMPARE(cfg.inputMethodProgram(), QStringLiteral("fcitx5"));
        QVERIFY(!cfg.debugEnabled());

        QSignalSpy generalSpy(&cfg, &Config::generalConfigChanged);
        cfg.setFocusPolicy(QStringLiteral("FollowMouse"));
        QCOMPARE(cfg.focusPolicy(), QStringLiteral("FollowMouse"));
        QCOMPARE(generalSpy.count(), 1);
    }

    void testWindowModelStackingAndFocus()
    {
        WindowModel model;
        QCOMPARE(model.count(), 0);
        QCOMPARE(model.activeAppName(), QStringLiteral("Tape"));

        Window* w1 = model.addMockWindow(QStringLiteral("Terminal — bash"), QStringLiteral("Konsole"));
        Window* w2 = model.addMockWindow(QStringLiteral("README.md — Kate"), QStringLiteral("Kate"));

        QCOMPARE(model.count(), 2);
        QCOMPARE(model.activeWindow(), w2);
        QCOMPARE(model.activeAppName(), QStringLiteral("Kate"));
        QVERIFY(!w1->isActivated());
        QVERIFY(w2->isActivated());
        QVERIFY(w2->zOrder() > w1->zOrder());

        // Click-to-focus w1 raises it above w2
        model.activateWindow(w1);
        QCOMPARE(model.activeWindow(), w1);
        QCOMPARE(model.activeAppName(), QStringLiteral("Konsole"));
        QVERIFY(w1->zOrder() > w2->zOrder());

        // Minimizing active window w1 shifts focus back to w2
        model.minimizeActiveWindow();
        QVERIFY(w1->isMinimized());
        QCOMPARE(model.activeWindow(), w2);

        // Maximizing w2 updates geometry and restores on toggle
        const QRectF initialGeom = w2->geometry();
        model.maximizeActiveWindow(0, 28, 1280, 704);
        QVERIFY(w2->isMaximized());
        QCOMPARE(w2->geometry(), QRectF(0, 28, 1280, 704));

        model.maximizeActiveWindow(0, 28, 1280, 704);
        QVERIFY(!w2->isMaximized());
        QCOMPARE(w2->geometry().topLeft(), initialGeom.topLeft());

        // Closing w2 leaves w1 (still minimized) so no active window
        model.closeWindow(w2);
        QCOMPARE(model.count(), 1);
        QCOMPARE(model.activeWindow(), nullptr);
    }
};

} // namespace Tape

QTEST_GUILESS_MAIN(Tape::TestConfigAndLogging)
#include "test_config_and_logging.moc"
