// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Application.h"
#include "core/Logging.h"

#include <QCoreApplication>
#include <QProcess>
#include <QProcessEnvironment>

#include <csignal>
#include <sys/socket.h>
#include <unistd.h>

namespace Tape {

int Application::s_sigFd[2] = { -1, -1 };

Application::Application(const CommandLineOptions& options, QObject* parent)
    : QObject(parent)
    , m_options(options)
{
}

Application::~Application()
{
    qCInfo(lcCore, "Shutting down Tape compositor");
    m_compositor.reset();
    m_config.reset();
    LogManager::instance()->shutdown();

    if (s_sigFd[0] >= 0) {
        ::close(s_sigFd[0]);
        s_sigFd[0] = -1;
    }
    if (s_sigFd[1] >= 0) {
        ::close(s_sigFd[1]);
        s_sigFd[1] = -1;
    }
}

bool Application::initialize(const QString& customLogDir)
{
    LogManager::instance()->initialize(customLogDir);

    m_config = std::make_unique<Config>(m_options.configPath);

    const bool enableDebug = m_options.debug || m_config->debugEnabled();
    const QStringList debugCats = !m_options.debugCategories.isEmpty()
        ? m_options.debugCategories
        : m_config->debugCategories();
    LogManager::instance()->setRules(enableDebug, debugCats);

    qCInfo(lcCore, "Starting Tape v%s (Qt %s)", TAPE_VERSION_STRING, qVersion());

    setupUnixSignalHandlers();

    m_compositor = std::make_unique<Compositor>(m_config.get());
    if (m_options.headless) {
        m_compositor->setUseHardwareIntegrationExtension(false);
    }
    if (!m_options.socketName.isEmpty()) {
        m_compositor->setSocketName(m_options.socketName.toUtf8());
    }

    m_compositor->create();
    if (!m_compositor->isCreated()) {
        qCCritical(lcCore, "Failed to initialize Wayland compositor");
        return false;
    }

    const bool enableProtoLog = m_options.protocolLogging || m_config->protocolLogging();
    if (enableProtoLog) {
        LogManager::instance()->setProtocolLogging(true, m_compositor->display());
    }

    if (!m_compositor->createOutputs(m_options.outputCount, m_options.outputSpec, m_options.headless)) {
        qCCritical(lcCore, "Failed to create compositor outputs");
        return false;
    }

    if (!m_options.startupCommand.isEmpty()) {
        launchStartupCommand();
    }

    return true;
}

void Application::unixSignalHandler(int sig)
{
    if (s_sigFd[0] >= 0) {
        const char sigByte = static_cast<char>(sig);
        const ssize_t written = ::write(s_sigFd[0], &sigByte, sizeof(sigByte));
        (void)written;
    }
}

void Application::setupUnixSignalHandlers()
{
    if (::socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, s_sigFd) != 0) {
        qCWarning(lcCore, "Failed to create signal socketpair");
        return;
    }

    m_sigNotifier = new QSocketNotifier(s_sigFd[1], QSocketNotifier::Read, this);
    connect(m_sigNotifier, &QSocketNotifier::activated, this, &Application::handleUnixSignal);

    struct sigaction sa {};
    sa.sa_handler = Application::unixSignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGHUP, &sa, nullptr);
}

void Application::handleUnixSignal()
{
    m_sigNotifier->setEnabled(false);
    char sigByte = 0;
    const ssize_t bytesRead = ::read(s_sigFd[1], &sigByte, sizeof(sigByte));
    (void)bytesRead;

    const int sig = static_cast<int>(sigByte);
    if (sig == SIGHUP) {
        qCInfo(lcCore, "Received SIGHUP — reloading configuration");
        if (m_config) {
            m_config->reload();
        }
    } else {
        qCInfo(lcCore, "Received signal %d — initiating clean shutdown", sig);
        QCoreApplication::quit();
    }

    m_sigNotifier->setEnabled(true);
}

void Application::launchStartupCommand()
{
    QProcess proc;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("WAYLAND_DISPLAY"), m_compositor->socketNameString());
    env.insert(QStringLiteral("XDG_SESSION_TYPE"), QStringLiteral("wayland"));
    env.insert(QStringLiteral("XDG_CURRENT_DESKTOP"), QStringLiteral("Tape"));
    env.insert(QStringLiteral("TAPE_COMPOSITOR_PID"),
        QString::number(QCoreApplication::applicationPid()));
    proc.setProcessEnvironment(env);
    proc.setProgram(QStringLiteral("/bin/sh"));
    proc.setArguments({ QStringLiteral("-c"), m_options.startupCommand });

    qint64 pid = 0;
    if (proc.startDetached(&pid)) {
        qCInfo(lcCore, "Launched startup command '%s' (PID %lld) on WAYLAND_DISPLAY=%s",
            qPrintable(m_options.startupCommand),
            static_cast<long long>(pid),
            qPrintable(m_compositor->socketNameString()));
    } else {
        qCWarning(lcCore, "Failed to launch startup command '%s'",
            qPrintable(m_options.startupCommand));
    }
}

} // namespace Tape
