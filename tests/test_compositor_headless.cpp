// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Application.h"
#include "core/CommandLine.h"
#include "core/Logging.h"

#include <QCoreApplication>
#include <QSet>
#include <QSocketNotifier>
#include <QTemporaryDir>
#include <QtTest/QtTest>

#include <poll.h>
#include <wayland-client.h>

namespace Tape {

namespace {

struct ClientRegistryState {
    QSet<QString> interfaces;
};

void registryGlobalHandler(void* data, struct wl_registry* registry,
    uint32_t name, const char* interface, uint32_t version)
{
    Q_UNUSED(registry)
    Q_UNUSED(name)
    Q_UNUSED(version)
    auto* state = static_cast<ClientRegistryState*>(data);
    state->interfaces.insert(QString::fromUtf8(interface));
}

void registryGlobalRemoveHandler(void* data, struct wl_registry* registry, uint32_t name)
{
    Q_UNUSED(data)
    Q_UNUSED(registry)
    Q_UNUSED(name)
}

const struct wl_registry_listener kRegistryListener = {
    registryGlobalHandler,
    registryGlobalRemoveHandler,
};

} // namespace

class TestCompositorHeadless : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testHeadlessWaylandServerAndProtocols()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString socketName = QStringLiteral("tape-test-%1")
                                       .arg(QCoreApplication::applicationPid());

        const QStringList args = {
            QStringLiteral("tape"),
            QStringLiteral("--headless"),
            QStringLiteral("--outputs"),
            QStringLiteral("2"),
            QStringLiteral("--output-size"),
            QStringLiteral("1280x800@1.0"),
            QStringLiteral("--socket"),
            socketName,
            QStringLiteral("--debug"),
            QStringLiteral("--protocol-log"),
            QStringLiteral("--config"),
            tempDir.filePath(QStringLiteral("taperc")),
        };

        const CommandLineOptions opts = CommandLine::parse(args);
        QVERIFY(opts.parseOk);
        QVERIFY(opts.headless);

        Application app(opts);
        QVERIFY(app.initialize(tempDir.path()));
        QVERIFY(app.compositor() != nullptr);
        QVERIFY(app.compositor()->isCreated());
        QCOMPARE(app.compositor()->socketNameString(), socketName);
        QCOMPARE(app.compositor()->quickOutputs().size(), 2);
        QVERIFY(LogManager::instance()->isProtocolLoggingEnabled());

        // Connect a real Wayland client to the headless compositor
        struct wl_display* clientDisplay = wl_display_connect(socketName.toUtf8().constData());
        QVERIFY2(clientDisplay != nullptr, "Failed to connect Wayland client to Tape socket");

        struct wl_registry* registry = wl_display_get_registry(clientDisplay);
        QVERIFY(registry != nullptr);

        ClientRegistryState regState;
        wl_registry_add_listener(registry, &kRegistryListener, &regState);

        // Flush client request and pump compositor event loop while reading client events
        wl_display_flush(clientDisplay);
        for (int i = 0; i < 20; ++i) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
            while (wl_display_prepare_read(clientDisplay) != 0) {
                wl_display_dispatch_pending(clientDisplay);
            }
            struct pollfd pfd { wl_display_get_fd(clientDisplay), POLLIN, 0 };
            if (::poll(&pfd, 1, 5) > 0) {
                wl_display_read_events(clientDisplay);
            } else {
                wl_display_cancel_read(clientDisplay);
            }
            wl_display_dispatch_pending(clientDisplay);
        }

        // Verify expected core & extension Wayland globals are advertised
        QVERIFY2(regState.interfaces.contains(QStringLiteral("wl_compositor")), "Missing wl_compositor");
        QVERIFY2(regState.interfaces.contains(QStringLiteral("wl_subcompositor")), "Missing wl_subcompositor");
        QVERIFY2(regState.interfaces.contains(QStringLiteral("wl_shm")), "Missing wl_shm");
        QVERIFY2(regState.interfaces.contains(QStringLiteral("wl_seat")), "Missing wl_seat");
        QVERIFY2(regState.interfaces.contains(QStringLiteral("wl_output")), "Missing wl_output");
        QVERIFY2(regState.interfaces.contains(QStringLiteral("xdg_wm_base")), "Missing xdg_wm_base");
        QVERIFY2(regState.interfaces.contains(QStringLiteral("zxdg_decoration_manager_v1")),
            "Missing zxdg_decoration_manager_v1");
        QVERIFY2(regState.interfaces.contains(QStringLiteral("zxdg_output_manager_v1")),
            "Missing zxdg_output_manager_v1");
        QVERIFY2(regState.interfaces.contains(QStringLiteral("wp_viewporter")), "Missing wp_viewporter");
        QVERIFY2(regState.interfaces.contains(QStringLiteral("zwp_idle_inhibit_manager_v1")),
            "Missing zwp_idle_inhibit_manager_v1");

        wl_registry_destroy(registry);
        wl_display_disconnect(clientDisplay);
    }
};

} // namespace Tape

int main(int argc, char* argv[])
{
    qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    QGuiApplication app(argc, argv);
    Tape::TestCompositorHeadless tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_compositor_headless.moc"
