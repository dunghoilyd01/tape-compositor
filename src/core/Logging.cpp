// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Logging.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QThread>

#include <cstdio>
#include <wayland-server-core.h>

Q_LOGGING_CATEGORY(lcCore, "tape.core")
Q_LOGGING_CATEGORY(lcDrm, "tape.drm")
Q_LOGGING_CATEGORY(lcSession, "tape.session")
Q_LOGGING_CATEGORY(lcInput, "tape.input")
Q_LOGGING_CATEGORY(lcOutput, "tape.output")
Q_LOGGING_CATEGORY(lcWm, "tape.wm")
Q_LOGGING_CATEGORY(lcXwayland, "tape.xwayland")
Q_LOGGING_CATEGORY(lcProtocol, "tape.protocol")
Q_LOGGING_CATEGORY(lcEffects, "tape.effects")
Q_LOGGING_CATEGORY(lcCapture, "tape.capture")
Q_LOGGING_CATEGORY(lcPortal, "tape.portal")
Q_LOGGING_CATEGORY(lcIm, "tape.im")
Q_LOGGING_CATEGORY(lcTheme, "tape.theme")
Q_LOGGING_CATEGORY(lcShell, "tape.shell")
Q_LOGGING_CATEGORY(lcNotify, "tape.notify")
Q_LOGGING_CATEGORY(lcPower, "tape.power")
Q_LOGGING_CATEGORY(lcDbus, "tape.dbus")

namespace Tape {

namespace {

constexpr qint64 kMaxLogFileBytes = 5LL * 1024 * 1024; // 5 MB per file
constexpr int kMaxRotatedFiles = 5;

void tapeMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    LogManager::instance()->writeMessage(type, context, msg);
}

void waylandProtocolLoggerFunc(void* userData, enum wl_protocol_logger_type direction,
    const struct wl_protocol_logger_message* message)
{
    Q_UNUSED(userData)
    const char* dirStr = (direction == WL_PROTOCOL_LOGGER_REQUEST) ? "->" : "<-";
    const char* objName = (message && message->resource) ? wl_resource_get_class(message->resource) : "unknown";
    const uint32_t objId = (message && message->resource) ? wl_resource_get_id(message->resource) : 0;
    const char* msgName = (message && message->message) ? message->message->name : "unknown";
    qCDebug(lcProtocol, "[wl %s] %s@%u.%s", dirStr, objName, objId, msgName);
}

const char* severityLabel(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return "DBG";
    case QtInfoMsg:
        return "INF";
    case QtWarningMsg:
        return "WRN";
    case QtCriticalMsg:
        return "ERR";
    case QtFatalMsg:
        return "FTL";
    }
    return "UNK";
}

} // namespace

LogManager* LogManager::instance()
{
    static LogManager s_instance;
    return &s_instance;
}

LogManager::LogManager(QObject* parent)
    : QObject(parent)
{
}

LogManager::~LogManager()
{
    shutdown();
}

void LogManager::initialize(const QString& customLogDir)
{
    {
        QMutexLocker locker(&m_mutex);
        if (m_initialized) {
            return;
        }

        QString stateDir = customLogDir;
        if (stateDir.isEmpty()) {
            const QString xdgState = qEnvironmentVariable("XDG_STATE_HOME");
            if (!xdgState.isEmpty()) {
                stateDir = xdgState + QStringLiteral("/tape");
            } else {
                stateDir = QDir::homePath() + QStringLiteral("/.local/state/tape");
            }
        }

        QDir dir(stateDir);
        if (!dir.exists()) {
            dir.mkpath(QStringLiteral("."));
        }

        m_logFilePath = dir.filePath(QStringLiteral("tape.log"));
        const QString oldLogPath = dir.filePath(QStringLiteral("tape.log.old"));

        if (QFile::exists(m_logFilePath)) {
            QFile::remove(oldLogPath);
            QFile::rename(m_logFilePath, oldLogPath);
        }

        m_logFile.setFileName(m_logFilePath);
        (void)m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        m_initialized = true;
    }

    setRules(false, {});
    qInstallMessageHandler(tapeMessageHandler);
}

void LogManager::shutdown()
{
    setProtocolLogging(false, nullptr);
    qInstallMessageHandler(nullptr);

    QMutexLocker locker(&m_mutex);
    if (m_logFile.isOpen()) {
        m_logFile.flush();
        m_logFile.close();
    }
    m_initialized = false;
}

bool LogManager::isDebugEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_debugEnabled;
}

QStringList LogManager::activeCategories() const
{
    QMutexLocker locker(&m_mutex);
    return m_categories;
}

bool LogManager::isProtocolLoggingEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_protocolLogger != nullptr;
}

QString LogManager::logFilePath() const
{
    QMutexLocker locker(&m_mutex);
    return m_logFilePath;
}

QStringList LogManager::allCategories()
{
    return {
        QStringLiteral("core"),
        QStringLiteral("drm"),
        QStringLiteral("session"),
        QStringLiteral("input"),
        QStringLiteral("output"),
        QStringLiteral("wm"),
        QStringLiteral("xwayland"),
        QStringLiteral("protocol"),
        QStringLiteral("effects"),
        QStringLiteral("capture"),
        QStringLiteral("portal"),
        QStringLiteral("im"),
        QStringLiteral("theme"),
        QStringLiteral("shell"),
        QStringLiteral("notify"),
        QStringLiteral("power"),
        QStringLiteral("dbus"),
    };
}

void LogManager::setRules(bool debugEnabled, const QStringList& categories)
{
    QStringList normalizedCats;
    for (const QString& cat : categories) {
        const QString trimmed = cat.trimmed();
        if (!trimmed.isEmpty()) {
            normalizedCats.append(trimmed.startsWith(QStringLiteral("tape."))
                    ? trimmed.mid(5)
                    : trimmed);
        }
    }

    QStringList rules;
    // Default baseline per PLAN §16: warning+ for all, info for core/session/output
    rules << QStringLiteral("tape.*.debug=false");
    rules << QStringLiteral("tape.*.info=false");
    rules << QStringLiteral("tape.core.info=true");
    rules << QStringLiteral("tape.session.info=true");
    rules << QStringLiteral("tape.output.info=true");

    if (debugEnabled) {
        if (normalizedCats.isEmpty()) {
            rules << QStringLiteral("tape.*.debug=true");
            rules << QStringLiteral("tape.*.info=true");
        } else {
            for (const QString& shortCat : normalizedCats) {
                rules << QStringLiteral("tape.%1.debug=true").arg(shortCat);
                rules << QStringLiteral("tape.%1.info=true").arg(shortCat);
            }
        }
    }

    QLoggingCategory::setFilterRules(rules.join(QLatin1Char('\n')));

    bool changedDebug = false;
    bool changedCats = false;
    {
        QMutexLocker locker(&m_mutex);
        changedDebug = (m_debugEnabled != debugEnabled);
        changedCats = (m_categories != normalizedCats);
        m_debugEnabled = debugEnabled;
        m_categories = normalizedCats;
    }

    if (changedDebug) {
        Q_EMIT debugEnabledChanged(debugEnabled);
    }
    if (changedCats) {
        Q_EMIT activeCategoriesChanged(normalizedCats);
    }

    qCInfo(lcCore, "Logging rules updated: debug=%s categories=[%s]",
        debugEnabled ? "ON" : "OFF",
        qPrintable(normalizedCats.isEmpty() ? QStringLiteral("all") : normalizedCats.join(QLatin1Char(','))));
}

void LogManager::setDebugEnabled(bool enabled)
{
    setRules(enabled, activeCategories());
}

void LogManager::toggleDebug()
{
    setDebugEnabled(!isDebugEnabled());
}

void LogManager::setProtocolLogging(bool enabled, struct wl_display* display)
{
    bool stateChanged = false;
    {
        QMutexLocker locker(&m_mutex);
        if (m_protocolLogger != nullptr) {
            wl_protocol_logger_destroy(m_protocolLogger);
            m_protocolLogger = nullptr;
            stateChanged = true;
        }
        if (enabled && display != nullptr) {
            m_protocolLogger = wl_display_add_protocol_logger(display, waylandProtocolLoggerFunc, nullptr);
            stateChanged = (m_protocolLogger != nullptr);
        }
    }

    if (enabled && !isDebugEnabled()) {
        // Ensure tape.protocol.debug is visible when protocol logging is enabled
        QStringList cats = activeCategories();
        if (!cats.contains(QStringLiteral("protocol"))) {
            cats.append(QStringLiteral("protocol"));
        }
        setRules(true, cats);
    }

    if (stateChanged) {
        Q_EMIT protocolLoggingEnabledChanged(enabled);
    }
}

void LogManager::rotateIfNeededLocked()
{
    if (!m_logFile.isOpen() || m_logFile.size() < kMaxLogFileBytes) {
        return;
    }

    m_logFile.flush();
    m_logFile.close();

    for (int i = kMaxRotatedFiles - 1; i >= 1; --i) {
        const QString src = QStringLiteral("%1.%2").arg(m_logFilePath).arg(i);
        const QString dst = QStringLiteral("%1.%2").arg(m_logFilePath).arg(i + 1);
        if (QFile::exists(src)) {
            QFile::remove(dst);
            QFile::rename(src, dst);
        }
    }

    const QString firstRot = QStringLiteral("%1.1").arg(m_logFilePath);
    QFile::remove(firstRot);
    QFile::rename(m_logFilePath, firstRot);

    (void)m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
}

void LogManager::writeMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
    const char* category = (context.category != nullptr) ? context.category : "default";
    const quintptr threadId = reinterpret_cast<quintptr>(QThread::currentThreadId());

    const QString formatted = QStringLiteral("[%1] [%2] [%3] [0x%4] %5\n")
                                  .arg(timestamp,
                                      QLatin1String(severityLabel(type)),
                                      QLatin1String(category),
                                      QString::number(threadId, 16),
                                      msg);

    const QByteArray utf8 = formatted.toUtf8();
    std::fwrite(utf8.constData(), 1, static_cast<size_t>(utf8.size()), stderr);
    if (type >= QtWarningMsg) {
        std::fflush(stderr);
    }

    QMutexLocker locker(&m_mutex);
    if (m_logFile.isOpen()) {
        rotateIfNeededLocked();
        m_logFile.write(utf8);
        m_logFile.flush();
    }
}

} // namespace Tape
