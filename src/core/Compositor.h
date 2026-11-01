// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/CommandLine.h"
#include "core/Config.h"
#include "core/Logging.h"
#include "wm/WindowModel.h"

#include <QList>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QtWaylandCompositor/QWaylandIdleInhibitManagerV1>
#include <QtWaylandCompositor/QWaylandQuickCompositor>
#include <QtWaylandCompositor/QWaylandQuickOutput>
#include <QtWaylandCompositor/QWaylandViewporter>
#include <QtWaylandCompositor/QWaylandXdgDecorationManagerV1>
#include <QtWaylandCompositor/QWaylandXdgOutputManagerV1>
#include <QtWaylandCompositor/QWaylandXdgShell>

namespace Tape {

class Compositor : public QWaylandQuickCompositor {
    Q_OBJECT
    Q_PROPERTY(Tape::WindowModel* windowModel READ windowModel CONSTANT)
    Q_PROPERTY(Tape::Config* config READ config CONSTANT)
    Q_PROPERTY(Tape::LogManager* logManager READ logManager CONSTANT)
    Q_PROPERTY(QString socketNameString READ socketNameString NOTIFY socketNameStringChanged)

public:
    explicit Compositor(Config* config, QObject* parent = nullptr);
    ~Compositor() override;

    void create() override;
    bool createOutputs(int count, const OutputSpec& spec, bool headless);

    WindowModel* windowModel() const { return m_windowModel; }
    Config* config() const { return m_config; }
    LogManager* logManager() const { return LogManager::instance(); }
    QString socketNameString() const { return QString::fromUtf8(socketName()); }

    QWaylandXdgShell* xdgShell() const { return m_xdgShell; }
    QList<QWaylandQuickOutput*> quickOutputs() const { return m_outputs; }
    QList<QQuickWindow*> outputWindows() const { return m_outputWindows; }

    Q_INVOKABLE void toggleDebugLogging();

Q_SIGNALS:
    void socketNameStringChanged();

private Q_SLOTS:
    void onToplevelCreated(QWaylandXdgToplevel* toplevel, QWaylandXdgSurface* xdgSurface);

private:
    Config* m_config = nullptr;
    WindowModel* m_windowModel = nullptr;
    QQmlEngine* m_qmlEngine = nullptr;

    QWaylandXdgShell* m_xdgShell = nullptr;
    QWaylandXdgDecorationManagerV1* m_decorationManager = nullptr;
    QWaylandXdgOutputManagerV1* m_xdgOutputManager = nullptr;
    QWaylandViewporter* m_viewporter = nullptr;
    QWaylandIdleInhibitManagerV1* m_idleInhibitManager = nullptr;

    QList<QQuickWindow*> m_outputWindows;
    QList<QWaylandQuickOutput*> m_outputs;
};

} // namespace Tape
