// SPDX-License-Identifier: GPL-3.0-or-later
#include "wm/Window.h"
#include "core/Logging.h"

namespace Tape {

quint64 Window::s_nextWindowId = 1;

Window::Window(QWaylandXdgToplevel* toplevel, QWaylandXdgSurface* xdgSurface, QObject* parent)
    : QObject(parent)
    , m_windowId(s_nextWindowId++)
    , m_toplevel(toplevel)
    , m_xdgSurface(xdgSurface)
{
    const qreal cascadeOffset = static_cast<qreal>((m_windowId - 1) % 8) * 32.0;
    m_geometry.moveTopLeft(QPointF(96.0 + cascadeOffset, 60.0 + cascadeOffset));
    m_savedGeometry = m_geometry;

    if (m_toplevel != nullptr) {
        connect(m_toplevel, &QWaylandXdgToplevel::titleChanged, this, &Window::titleChanged);
        connect(m_toplevel, &QWaylandXdgToplevel::appIdChanged, this, &Window::appIdChanged);
        connect(m_toplevel, &QWaylandXdgToplevel::decorationModeChanged, this, &Window::serverDecoratedChanged);
        connect(m_toplevel, &QWaylandXdgToplevel::setMinimized, this, [this]() {
            setMinimized(true);
        });
    }

    if (m_xdgSurface != nullptr) {
        connect(m_xdgSurface, &QWaylandXdgSurface::windowGeometryChanged, this, &Window::geometryChanged);
        if (m_xdgSurface->surface() != nullptr) {
            connect(m_xdgSurface->surface(), &QWaylandSurface::hasContentChanged, this, [this]() {
                if (m_xdgSurface != nullptr && m_xdgSurface->surface() != nullptr
                    && !m_xdgSurface->surface()->hasContent()) {
                    Q_EMIT closed(this);
                }
            });
            connect(m_xdgSurface->surface(), &QObject::destroyed, this, [this]() {
                Q_EMIT closed(this);
            });
        }
    }

    qCDebug(lcWm, "Created Window id=%llu appId='%s' title='%s'",
        static_cast<unsigned long long>(m_windowId),
        qPrintable(appId()),
        qPrintable(title()));
}

Window::Window(const QString& mockTitle, const QString& mockAppId, QObject* parent)
    : QObject(parent)
    , m_windowId(s_nextWindowId++)
    , m_fallbackTitle(mockTitle)
    , m_fallbackAppId(mockAppId)
{
}

QString Window::title() const
{
    if (m_toplevel != nullptr && !m_toplevel->title().isEmpty()) {
        return m_toplevel->title();
    }
    if (!m_fallbackTitle.isEmpty()) {
        return m_fallbackTitle;
    }
    return appId();
}

QString Window::appId() const
{
    if (m_toplevel != nullptr && !m_toplevel->appId().isEmpty()) {
        return m_toplevel->appId();
    }
    if (!m_fallbackAppId.isEmpty()) {
        return m_fallbackAppId;
    }
    return QStringLiteral("Application");
}

QWaylandSurface* Window::surface() const
{
    return (m_xdgSurface != nullptr) ? m_xdgSurface->surface() : nullptr;
}

qreal Window::width() const
{
    if (m_xdgSurface != nullptr && m_xdgSurface->windowGeometry().width() > 0) {
        return m_xdgSurface->windowGeometry().width();
    }
    if (surface() != nullptr && surface()->destinationSize().width() > 0) {
        return surface()->destinationSize().width();
    }
    return m_geometry.width();
}

qreal Window::height() const
{
    if (m_xdgSurface != nullptr && m_xdgSurface->windowGeometry().height() > 0) {
        return m_xdgSurface->windowGeometry().height();
    }
    if (surface() != nullptr && surface()->destinationSize().height() > 0) {
        return surface()->destinationSize().height();
    }
    return m_geometry.height();
}

void Window::setX(qreal newX)
{
    if (qFuzzyCompare(m_geometry.x(), newX)) {
        return;
    }
    m_geometry.moveLeft(newX);
    Q_EMIT geometryChanged();
}

void Window::setY(qreal newY)
{
    if (qFuzzyCompare(m_geometry.y(), newY)) {
        return;
    }
    m_geometry.moveTop(newY);
    Q_EMIT geometryChanged();
}

void Window::setPosition(const QPointF& pos)
{
    if (m_geometry.topLeft() == pos) {
        return;
    }
    m_geometry.moveTopLeft(pos);
    Q_EMIT geometryChanged();
}

void Window::setZOrder(int z)
{
    if (m_zOrder == z) {
        return;
    }
    m_zOrder = z;
    Q_EMIT zOrderChanged();
}

void Window::setActivated(bool active)
{
    if (m_activated == active) {
        return;
    }
    m_activated = active;
    if (m_toplevel != nullptr) {
        QList<QWaylandXdgToplevel::State> states;
        if (m_maximized) {
            states.append(QWaylandXdgToplevel::MaximizedState);
        }
        if (m_fullscreen) {
            states.append(QWaylandXdgToplevel::FullscreenState);
        }
        if (m_activated) {
            states.append(QWaylandXdgToplevel::ActivatedState);
        }
        m_toplevel->sendConfigure(QSize(0, 0), states);
    }
    Q_EMIT activatedChanged();
}

void Window::setMinimized(bool minimized)
{
    if (m_minimized == minimized) {
        return;
    }
    m_minimized = minimized;
    qCDebug(lcWm, "Window id=%llu minimized=%s",
        static_cast<unsigned long long>(m_windowId),
        minimized ? "true" : "false");
    Q_EMIT minimizedChanged();
}

void Window::setMaximizedState(bool maximized, const QRectF& targetGeometry)
{
    if (m_maximized == maximized) {
        return;
    }
    if (maximized) {
        m_savedGeometry = QRectF(m_geometry.topLeft(), QSizeF(width(), height()));
        if (targetGeometry.isValid()) {
            m_geometry = targetGeometry;
        }
        m_maximized = true;
        if (m_toplevel != nullptr) {
            m_toplevel->sendMaximized(m_geometry.size().toSize());
        }
    } else {
        m_maximized = false;
        m_geometry = m_savedGeometry;
        if (m_toplevel != nullptr) {
            m_toplevel->sendUnmaximized(m_savedGeometry.size().toSize());
        }
    }
    Q_EMIT maximizedChanged();
    Q_EMIT geometryChanged();
}

bool Window::isServerDecorated() const
{
    if (m_toplevel == nullptr) {
        return true;
    }
    return m_toplevel->decorationMode() != QWaylandXdgToplevel::ClientSideDecoration;
}

void Window::requestClose()
{
    qCDebug(lcWm, "Requesting close for Window id=%llu", static_cast<unsigned long long>(m_windowId));
    if (m_toplevel != nullptr) {
        m_toplevel->sendClose();
    } else {
        Q_EMIT closed(this);
    }
}

} // namespace Tape
