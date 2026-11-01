// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QFile>
#include <QLoggingCategory>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>

struct wl_display;
struct wl_protocol_logger;

// Logging categories defined in PLAN §16
Q_DECLARE_LOGGING_CATEGORY(lcCore)
Q_DECLARE_LOGGING_CATEGORY(lcDrm)
Q_DECLARE_LOGGING_CATEGORY(lcSession)
Q_DECLARE_LOGGING_CATEGORY(lcInput)
Q_DECLARE_LOGGING_CATEGORY(lcOutput)
Q_DECLARE_LOGGING_CATEGORY(lcWm)
Q_DECLARE_LOGGING_CATEGORY(lcXwayland)
Q_DECLARE_LOGGING_CATEGORY(lcProtocol)
Q_DECLARE_LOGGING_CATEGORY(lcEffects)
Q_DECLARE_LOGGING_CATEGORY(lcCapture)
Q_DECLARE_LOGGING_CATEGORY(lcPortal)
Q_DECLARE_LOGGING_CATEGORY(lcIm)
Q_DECLARE_LOGGING_CATEGORY(lcTheme)
Q_DECLARE_LOGGING_CATEGORY(lcShell)
Q_DECLARE_LOGGING_CATEGORY(lcNotify)
Q_DECLARE_LOGGING_CATEGORY(lcPower)
Q_DECLARE_LOGGING_CATEGORY(lcDbus)

namespace Tape {

class LogManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool debugEnabled READ isDebugEnabled WRITE setDebugEnabled NOTIFY debugEnabledChanged)
    Q_PROPERTY(QStringList activeCategories READ activeCategories NOTIFY activeCategoriesChanged)
    Q_PROPERTY(bool protocolLoggingEnabled READ isProtocolLoggingEnabled NOTIFY protocolLoggingEnabledChanged)
    Q_PROPERTY(QString logFilePath READ logFilePath CONSTANT)

public:
    static LogManager* instance();

    void initialize(const QString& customLogDir = QString());
    void shutdown();

    bool isDebugEnabled() const;
    QStringList activeCategories() const;
    bool isProtocolLoggingEnabled() const;
    QString logFilePath() const;

    static QStringList allCategories();

    void setRules(bool debugEnabled, const QStringList& categories = {});
    Q_INVOKABLE void setDebugEnabled(bool enabled);
    Q_INVOKABLE void toggleDebug();

    void setProtocolLogging(bool enabled, struct wl_display* display);

    void writeMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg);

Q_SIGNALS:
    void debugEnabledChanged(bool enabled);
    void activeCategoriesChanged(const QStringList& categories);
    void protocolLoggingEnabledChanged(bool enabled);

private:
    explicit LogManager(QObject* parent = nullptr);
    ~LogManager() override;

    void rotateIfNeededLocked();

    mutable QMutex m_mutex;
    QFile m_logFile;
    QString m_logFilePath;
    bool m_initialized = false;
    bool m_debugEnabled = false;
    QStringList m_categories;
    struct wl_protocol_logger* m_protocolLogger = nullptr;
};

} // namespace Tape
