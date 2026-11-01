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
    , m_surface(xdgSurface != nullptr ? xdgSurface->surface() : nullptr)
{
    const qreal cascadeOffset = static_cast<qreal>((m_windowId - 1) % 8) * 32.0;
    m_geometry.moveTopLeft(QPointF(96.0 + cascadeOffset, 60.0 + cascadeOffset));
    m_savedGeometry = m_geometry;

    if (!m_toplevel.isNull()) {
        m_cachedTitle = m_toplevel->title();
        m_cachedAppId = m_toplevel->appId();

        connect(m_toplevel, &QWaylandXdgToplevel::titleChanged, this, [this]() {
            if (!m_toplevel.isNull()) {
                m_cachedTitle = m_toplevel->title();
            }
            Q_EMIT titleChanged();
        });
        connect(m_toplevel, &QWaylandXdgToplevel::appIdChanged, this, [this]() {
            if (!m_toplevel.isNull()) {
                m_cachedAppId = m_toplevel->appId();
            }
            Q_EMIT appIdChanged();
        });
        connect(m_toplevel, &QWaylandXdgToplevel::decorationModeChanged,
            this, &Window::serverDecoratedChanged);
        connect(m_toplevel, &QWaylandXdgToplevel::setMinimized, this, [this]() {
            setMinimized(true);
        });
        connect(m_toplevel, &QWaylandXdgToplevel::setMaximized, this, [this]() {
            Q_EMIT clientRequestedMaximize(true);
        });
        connect(m_toplevel, &QWaylandXdgToplevel::unsetMaximized, this, [this]() {
            Q_EMIT clientRequestedMaximize(false);
        });
        connect(m_toplevel, &QObject::destroyed, this, [this]() {
            notifyClosedOnce();
        });
    }

    if (!m_xdgSurface.isNull()) {
        connect(m_xdgSurface, &QWaylandXdgSurface::windowGeometryChanged,
            this, &Window::geometryChanged);
        connect(m_xdgSurface, &QObject::destroyed, this, [this]() {
            Q_EMIT shellSurfaceChanged();
            notifyClosedOnce();
        });
    }

    if (!m_surface.isNull()) {
        connect(m_surface, &QWaylandSurface::hasContentChanged, this, [this]() {
            if (!m_surface.isNull() && !m_surface->hasContent()) {
                notifyClosedOnce();
            }
        });
        connect(m_surface, &QObject::destroyed, this, [this]() {
            notifyClosedOnce();
        });
    }

    qCDebug(lcWm, "Created Window id=%llu appId='%s' title='%s'",
        static_cast<unsigned long long>(m_windowId),
        qPrintable(appId()),
        qPrintable(title()));
}

Window::Window(const QString& mockTitle, const QString& mockAppId, QObject* parent)
    : QObject(parent)
    , m_windowId(s_nextWindowId++)
    , m_cachedTitle(mockTitle)
    , m_cachedAppId(mockAppId)
{
}

void Window::notifyClosedOnce()
{
    if (m_closing) {
        return;
    }
    m_closing = true;
    qCDebug(lcWm, "Window id=%llu closed", static_cast<unsigned long long>(m_windowId));
    Q_EMIT closed(this);
}

QString Window::title() const
{
    if (!m_toplevel.isNull() && !m_toplevel->title().isEmpty()) {
        return m_toplevel->title();
    }
    if (!m_cachedTitle.isEmpty()) {
        return m_cachedTitle;
    }
    return appId();
}

QString Window::appId() const
{
    if (!m_toplevel.isNull() && !m_toplevel->appId().isEmpty()) {
        return m_toplevel->appId();
    }
    if (!m_cachedAppId.isEmpty()) {
        return m_cachedAppId;
    }
    return QStringLiteral("Application");
}

qreal Window::width() const
{
    if (!m_xdgSurface.isNull() && m_xdgSurface->windowGeometry().width() > 0) {
        return m_xdgSurface->windowGeometry().width();
    }
    if (!m_surface.isNull() && m_surface->destinationSize().width() > 0) {
        return m_surface->destinationSize().width();
    }
    return m_geometry.width();
}

qreal Window::height() const
{
    if (!m_xdgSurface.isNull() && m_xdgSurface->windowGeometry().height() > 0) {
        return m_xdgSurface->windowGeometry().height();
    }
    if (!m_surface.isNull() && m_surface->destinationSize().height() > 0) {
        return m_surface->destinationSize().height();
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
    if (!m_closing && !m_toplevel.isNull() && !m_xdgSurface.isNull()
        && !m_surface.isNull() && m_surface->hasContent()) {
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
        if (!m_closing && !m_toplevel.isNull()) {
            m_toplevel->sendMaximized(m_geometry.size().toSize());
        }
    } else {
        m_maximized = false;
        m_geometry = m_savedGeometry;
        if (!m_closing && !m_toplevel.isNull()) {
            m_toplevel->sendUnmaximized(m_savedGeometry.size().toSize());
        }
    }
    Q_EMIT maximizedChanged();
    Q_EMIT geometryChanged();
}

bool Window::isServerDecorated() const
{
    if (m_toplevel.isNull()) {
        return true;
    }
    return m_toplevel->decorationMode() != QWaylandXdgToplevel::ClientSideDecoration;
}

void Window::requestClose()
{
    qCDebug(lcWm, "Requesting close for Window id=%llu", static_cast<unsigned long long>(m_windowId));
    if (!m_toplevel.isNull()) {
        m_toplevel->sendClose();
    } else {
        notifyClosedOnce();
    }
}

} // namespace Tape
