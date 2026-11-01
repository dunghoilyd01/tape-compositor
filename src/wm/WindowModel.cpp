// SPDX-License-Identifier: GPL-3.0-or-later
#include "wm/WindowModel.h"
#include "core/Logging.h"

#include <QtWaylandCompositor/QWaylandSeat>

namespace Tape {

WindowModel::WindowModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int WindowModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_windows.size());
}

QVariant WindowModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_windows.size()) {
        return {};
    }

    Window* win = m_windows.at(index.row());
    switch (role) {
    case WindowObjectRole:
        return QVariant::fromValue(win);
    case WindowIdRole:
        return win->windowId();
    case TitleRole:
        return win->title();
    case AppIdRole:
        return win->appId();
    case ShellSurfaceRole:
        return QVariant::fromValue(win->shellSurface());
    case ActivatedRole:
        return win->isActivated();
    case MinimizedRole:
        return win->isMinimized();
    case MaximizedRole:
        return win->isMaximized();
    case ZOrderRole:
        return win->zOrder();
    default:
        return {};
    }
}

QHash<int, QByteArray> WindowModel::roleNames() const
{
    return {
        { WindowObjectRole, "windowObject" },
        { WindowIdRole, "windowId" },
        { TitleRole, "title" },
        { AppIdRole, "appId" },
        { ShellSurfaceRole, "shellSurface" },
        { ActivatedRole, "activated" },
        { MinimizedRole, "minimized" },
        { MaximizedRole, "maximized" },
        { ZOrderRole, "zOrder" },
    };
}

QString WindowModel::activeAppName() const
{
    if (m_activeWindow != nullptr) {
        return m_activeWindow->appId();
    }
    return QStringLiteral("Tape");
}

Window* WindowModel::addXdgToplevel(QWaylandXdgToplevel* toplevel, QWaylandXdgSurface* xdgSurface)
{
    auto* win = new Window(toplevel, xdgSurface, this);
    registerWindow(win);
    return win;
}

Window* WindowModel::addMockWindow(const QString& title, const QString& appId)
{
    auto* win = new Window(title, appId, this);
    registerWindow(win);
    return win;
}

void WindowModel::registerWindow(Window* window)
{
    const int row = static_cast<int>(m_windows.size());
    beginInsertRows(QModelIndex(), row, row);
    window->setZOrder(m_topZOrder++);
    m_windows.append(window);
    endInsertRows();

    connect(window, &Window::titleChanged, this, [this, window]() {
        notifyRowChanged(window, { TitleRole });
    });
    connect(window, &Window::appIdChanged, this, [this, window]() {
        notifyRowChanged(window, { AppIdRole });
        if (m_activeWindow == window) {
            Q_EMIT activeWindowChanged();
        }
    });
    connect(window, &Window::activatedChanged, this, [this, window]() {
        notifyRowChanged(window, { ActivatedRole });
    });
    connect(window, &Window::minimizedChanged, this, [this, window]() {
        notifyRowChanged(window, { MinimizedRole });
    });
    connect(window, &Window::maximizedChanged, this, [this, window]() {
        notifyRowChanged(window, { MaximizedRole });
    });
    connect(window, &Window::zOrderChanged, this, [this, window]() {
        notifyRowChanged(window, { ZOrderRole });
    });
    connect(window, &Window::closed, this, &WindowModel::removeWindow);

    Q_EMIT countChanged();
    Q_EMIT windowAdded(window);
    activateWindow(window);
}

void WindowModel::removeWindow(Window* window)
{
    const int row = indexOfWindow(window);
    if (row < 0) {
        return;
    }

    const bool wasActive = (m_activeWindow == window);

    beginRemoveRows(QModelIndex(), row, row);
    m_windows.removeAt(row);
    endRemoveRows();

    Q_EMIT countChanged();
    Q_EMIT windowRemoved(window);

    if (wasActive) {
        m_activeWindow = nullptr;
        Window* next = findTopmostVisibleWindow();
        if (next != nullptr) {
            activateWindow(next);
        } else {
            Q_EMIT activeWindowChanged();
        }
    }

    window->deleteLater();
}

void WindowModel::activateWindow(Window* window)
{
    if (window == nullptr) {
        if (m_activeWindow != nullptr) {
            m_activeWindow->setActivated(false);
            m_activeWindow = nullptr;
            Q_EMIT activeWindowChanged();
        }
        return;
    }

    if (window->isMinimized()) {
        window->setMinimized(false);
    }

    raiseWindow(window);

    if (m_activeWindow != window) {
        if (m_activeWindow != nullptr) {
            m_activeWindow->setActivated(false);
        }
        m_activeWindow = window;
        m_activeWindow->setActivated(true);
        Q_EMIT activeWindowChanged();
    } else if (!m_activeWindow->isActivated()) {
        m_activeWindow->setActivated(true);
    }

    if (m_seat != nullptr && window->surface() != nullptr) {
        m_seat->setKeyboardFocus(window->surface());
    }

    qCDebug(lcWm, "Activated window id=%llu (%s)",
        static_cast<unsigned long long>(window->windowId()),
        qPrintable(window->title()));
}

void WindowModel::raiseWindow(Window* window)
{
    if (window == nullptr || indexOfWindow(window) < 0) {
        return;
    }
    window->setZOrder(m_topZOrder++);
}

void WindowModel::toggleMinimize(Window* window)
{
    if (window == nullptr) {
        return;
    }
    const bool newMinimized = !window->isMinimized();
    window->setMinimized(newMinimized);

    if (newMinimized && m_activeWindow == window) {
        window->setActivated(false);
        m_activeWindow = nullptr;
        Window* next = findTopmostVisibleWindow();
        if (next != nullptr) {
            activateWindow(next);
        } else {
            Q_EMIT activeWindowChanged();
        }
    } else if (!newMinimized) {
        activateWindow(window);
    }
}

void WindowModel::toggleMaximize(Window* window, qreal availX, qreal availY, qreal availW, qreal availH)
{
    if (window == nullptr) {
        return;
    }
    activateWindow(window);
    window->setMaximizedState(!window->isMaximized(), QRectF(availX, availY, availW, availH));
}

void WindowModel::closeWindow(Window* window)
{
    if (window != nullptr) {
        window->requestClose();
    }
}

void WindowModel::closeActiveWindow()
{
    closeWindow(m_activeWindow);
}

void WindowModel::minimizeActiveWindow()
{
    toggleMinimize(m_activeWindow);
}

void WindowModel::maximizeActiveWindow(qreal availX, qreal availY, qreal availW, qreal availH)
{
    toggleMaximize(m_activeWindow, availX, availY, availW, availH);
}

Window* WindowModel::findTopmostVisibleWindow() const
{
    Window* best = nullptr;
    int bestZ = -1;
    for (Window* win : m_windows) {
        if (!win->isMinimized() && win->zOrder() > bestZ) {
            best = win;
            bestZ = win->zOrder();
        }
    }
    return best;
}

int WindowModel::indexOfWindow(Window* window) const
{
    return static_cast<int>(m_windows.indexOf(window));
}

void WindowModel::notifyRowChanged(Window* window, const QList<int>& roles)
{
    const int row = indexOfWindow(window);
    if (row >= 0) {
        const QModelIndex idx = index(row, 0);
        Q_EMIT dataChanged(idx, idx, roles);
    }
}

} // namespace Tape
