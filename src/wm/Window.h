// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QObject>
#include <QPointer>
#include <QRectF>
#include <QString>
#include <QtWaylandCompositor/QWaylandShellSurface>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandXdgSurface>
#include <QtWaylandCompositor/QWaylandXdgToplevel>

namespace Tape {

class Window : public QObject {
    Q_OBJECT
    Q_PROPERTY(quint64 windowId READ windowId CONSTANT)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString appId READ appId NOTIFY appIdChanged)
    Q_PROPERTY(QWaylandShellSurface* shellSurface READ shellSurface NOTIFY shellSurfaceChanged)
    Q_PROPERTY(QWaylandXdgToplevel* toplevel READ toplevel CONSTANT)
    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY geometryChanged)
    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY geometryChanged)
    Q_PROPERTY(qreal width READ width NOTIFY geometryChanged)
    Q_PROPERTY(qreal height READ height NOTIFY geometryChanged)
    Q_PROPERTY(int zOrder READ zOrder NOTIFY zOrderChanged)
    Q_PROPERTY(bool activated READ isActivated NOTIFY activatedChanged)
    Q_PROPERTY(bool minimized READ isMinimized WRITE setMinimized NOTIFY minimizedChanged)
    Q_PROPERTY(bool maximized READ isMaximized NOTIFY maximizedChanged)
    Q_PROPERTY(bool fullscreen READ isFullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(bool serverDecorated READ isServerDecorated NOTIFY serverDecoratedChanged)

public:
    explicit Window(QWaylandXdgToplevel* toplevel, QWaylandXdgSurface* xdgSurface, QObject* parent = nullptr);
    explicit Window(const QString& mockTitle, const QString& mockAppId, QObject* parent = nullptr);
    ~Window() override = default;

    quint64 windowId() const { return m_windowId; }
    QString title() const;
    QString appId() const;

    QWaylandShellSurface* shellSurface() const { return m_xdgSurface.data(); }
    QWaylandXdgSurface* xdgSurface() const { return m_xdgSurface.data(); }
    QWaylandXdgToplevel* toplevel() const { return m_toplevel.data(); }
    QWaylandSurface* surface() const { return m_surface.data(); }

    qreal x() const { return m_geometry.x(); }
    qreal y() const { return m_geometry.y(); }
    qreal width() const;
    qreal height() const;
    QRectF geometry() const { return m_geometry; }

    void setX(qreal newX);
    void setY(qreal newY);
    void setPosition(const QPointF& pos);

    int zOrder() const { return m_zOrder; }
    void setZOrder(int z);

    bool isActivated() const { return m_activated; }
    void setActivated(bool active);

    bool isMinimized() const { return m_minimized; }
    void setMinimized(bool minimized);

    bool isMaximized() const { return m_maximized; }
    void setMaximizedState(bool maximized, const QRectF& targetGeometry = QRectF());

    bool isFullscreen() const { return m_fullscreen; }
    bool isServerDecorated() const;

    Q_INVOKABLE void requestClose();

Q_SIGNALS:
    void titleChanged();
    void appIdChanged();
    void shellSurfaceChanged();
    void geometryChanged();
    void zOrderChanged();
    void activatedChanged();
    void minimizedChanged();
    void maximizedChanged();
    void fullscreenChanged();
    void serverDecoratedChanged();
    void clientRequestedMaximize(bool maximize);
    void closed(Tape::Window* window);

private:
    void notifyClosedOnce();

    static quint64 s_nextWindowId;

    quint64 m_windowId = 0;
    QPointer<QWaylandXdgToplevel> m_toplevel;
    QPointer<QWaylandXdgSurface> m_xdgSurface;
    QPointer<QWaylandSurface> m_surface;
    QString m_cachedTitle;
    QString m_cachedAppId;
    QRectF m_geometry { 120.0, 80.0, 720.0, 480.0 };
    QRectF m_savedGeometry { 120.0, 80.0, 720.0, 480.0 };
    int m_zOrder = 0;
    bool m_activated = false;
    bool m_minimized = false;
    bool m_maximized = false;
    bool m_fullscreen = false;
    bool m_closing = false;
};

} // namespace Tape
