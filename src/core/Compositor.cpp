// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Compositor.h"

#include <QQmlComponent>
#include <QQmlContext>
#include <QtWaylandCompositor/QWaylandOutputMode>
#include <QtWaylandCompositor/QWaylandXdgOutputV1>

#include <cmath>

namespace Tape {

Compositor::Compositor(Config* config, QObject* parent)
    : QWaylandQuickCompositor(parent)
    , m_config(config)
    , m_windowModel(new WindowModel(this))
    , m_qmlEngine(new QQmlEngine(this))
    , m_xdgShell(new QWaylandXdgShell(this))
    , m_decorationManager(new QWaylandXdgDecorationManagerV1())
    , m_xdgOutputManager(new QWaylandXdgOutputManagerV1(this))
    , m_viewporter(new QWaylandViewporter(this))
    , m_idleInhibitManager(new QWaylandIdleInhibitManagerV1(this))
{
    m_decorationManager->setParent(this);
    m_decorationManager->setExtensionContainer(this);
    m_decorationManager->setPreferredMode(QWaylandXdgToplevel::ServerSideDecoration);

    connect(m_xdgShell, &QWaylandXdgShell::toplevelCreated,
        this, &Compositor::onToplevelCreated);
    connect(this, &QWaylandCompositor::socketNameChanged,
        this, &Compositor::socketNameStringChanged);
}

Compositor::~Compositor()
{
    for (QQuickWindow* win : m_outputWindows) {
        delete win;
    }
    m_outputWindows.clear();
}

void Compositor::create()
{
    QWaylandQuickCompositor::create();

    m_xdgShell->initialize();
    m_decorationManager->initialize();
    m_xdgOutputManager->initialize();
    m_viewporter->initialize();
    m_idleInhibitManager->initialize();

    m_windowModel->setDefaultSeat(defaultSeat());

    qCInfo(lcCore, "Wayland compositor initialized on socket '%s'",
        socketName().constData());
}

bool Compositor::createOutputs(int count, const OutputSpec& spec, bool headless)
{
    QQmlComponent component(m_qmlEngine, QUrl(QStringLiteral("qrc:/qt/qml/Tape/Shell/src/qml/OutputWindow.qml")));
    if (component.isError()) {
        for (const QQmlError& err : component.errors()) {
            qCCritical(lcOutput, "QML load error: %s", qPrintable(err.toString()));
        }
        return false;
    }

    const int scaleInt = std::max(1, static_cast<int>(std::ceil(spec.scale)));

    for (int i = 0; i < count; ++i) {
        QVariantMap initialProps;
        initialProps.insert(QStringLiteral("compositor"), QVariant::fromValue(this));
        initialProps.insert(QStringLiteral("outputIndex"), i);
        initialProps.insert(QStringLiteral("width"), spec.size.width());
        initialProps.insert(QStringLiteral("height"), spec.size.height());
        initialProps.insert(QStringLiteral("title"),
            QStringLiteral("Tape Compositor — Output %1 (%2)").arg(i + 1).arg(socketNameString()));

        QObject* obj = component.createWithInitialProperties(initialProps);
        auto* quickWin = qobject_cast<QQuickWindow*>(obj);
        if (quickWin == nullptr) {
            qCCritical(lcOutput, "OutputWindow.qml root is not a QQuickWindow");
            delete obj;
            return false;
        }

        auto* output = new QWaylandQuickOutput(this, quickWin);
        output->setManufacturer(QStringLiteral("Tape"));
        output->setModel(headless ? QStringLiteral("Headless-%1").arg(i + 1)
                                  : QStringLiteral("Nested-%1").arg(i + 1));
        output->setPosition(QPoint(i * spec.size.width(), 0));
        output->setScaleFactor(scaleInt);
        output->setSizeFollowsWindow(true);

        const QWaylandOutputMode mode(spec.size, 60000);
        output->addMode(mode, true);
        output->setCurrentMode(mode);

        auto* xdgOutput = new QWaylandXdgOutputV1(output, m_xdgOutputManager);
        xdgOutput->setName(QStringLiteral("TAPE-%1").arg(i + 1));
        xdgOutput->setDescription(
            QStringLiteral("Tape %1 Output %2").arg(headless ? QStringLiteral("Headless") : QStringLiteral("Nested")).arg(i + 1));
        xdgOutput->setLogicalPosition(output->position());
        xdgOutput->setLogicalSize(spec.size);

        if (i == 0) {
            setDefaultOutput(output);
        }

        if (!headless) {
            quickWin->show();
        }

        m_outputWindows.append(quickWin);
        m_outputs.append(output);

        qCInfo(lcOutput, "Created output %s: %dx%d @ %.2fx (mode=%s)",
            qPrintable(xdgOutput->name()),
            spec.size.width(),
            spec.size.height(),
            spec.scale,
            headless ? "headless" : "nested");
    }

    return true;
}

void Compositor::toggleDebugLogging()
{
    LogManager::instance()->toggleDebug();
    if (m_config != nullptr) {
        m_config->setDebugEnabled(LogManager::instance()->isDebugEnabled());
    }
}

void Compositor::onToplevelCreated(QWaylandXdgToplevel* toplevel, QWaylandXdgSurface* xdgSurface)
{
    m_windowModel->addXdgToplevel(toplevel, xdgSurface);
}

} // namespace Tape
