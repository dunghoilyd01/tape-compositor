// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "wm/Window.h"

#include <QAbstractListModel>
#include <QList>
#include <QRectF>

class QWaylandSeat;

namespace Tape {

class WindowModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(Tape::Window* activeWindow READ activeWindow NOTIFY activeWindowChanged)
    Q_PROPERTY(QString activeAppName READ activeAppName NOTIFY activeWindowChanged)

public:
    enum Roles {
        WindowObjectRole = Qt::UserRole + 1,
        WindowIdRole,
        TitleRole,
        AppIdRole,
        ShellSurfaceRole,
        ActivatedRole,
        MinimizedRole,
        MaximizedRole,
        ZOrderRole
    };

    explicit WindowModel(QObject* parent = nullptr);
    ~WindowModel() override = default;

    void setDefaultSeat(QWaylandSeat* seat) { m_seat = seat; }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return static_cast<int>(m_windows.size()); }
    Window* activeWindow() const { return m_activeWindow; }
    QString activeAppName() const;

    Window* addXdgToplevel(QWaylandXdgToplevel* toplevel, QWaylandXdgSurface* xdgSurface);
    Window* addMockWindow(const QString& title, const QString& appId);

    Q_INVOKABLE void removeWindow(Tape::Window* window);
    Q_INVOKABLE void activateWindow(Tape::Window* window);
    Q_INVOKABLE void raiseWindow(Tape::Window* window);
    Q_INVOKABLE void toggleMinimize(Tape::Window* window);
    Q_INVOKABLE void toggleMaximize(Tape::Window* window, qreal availX, qreal availY, qreal availW, qreal availH);
    Q_INVOKABLE void closeWindow(Tape::Window* window);
    Q_INVOKABLE void closeActiveWindow();
    Q_INVOKABLE void minimizeActiveWindow();
    Q_INVOKABLE void maximizeActiveWindow(qreal availX, qreal availY, qreal availW, qreal availH);

    QList<Window*> windows() const { return m_windows; }

Q_SIGNALS:
    void countChanged();
    void activeWindowChanged();
    void windowAdded(Tape::Window* window);
    void windowRemoved(Tape::Window* window);

private:
    void registerWindow(Window* window);
    Window* findTopmostVisibleWindow() const;
    int indexOfWindow(Window* window) const;
    void notifyRowChanged(Window* window, const QList<int>& roles = {});

    QList<Window*> m_windows;
    Window* m_activeWindow = nullptr;
    QWaylandSeat* m_seat = nullptr;
    int m_topZOrder = 1;
};

} // namespace Tape
