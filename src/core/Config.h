// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <KConfigGroup>
#include <KConfigWatcher>
#include <KSharedConfig>
#include <QColor>
#include <QObject>
#include <QString>
#include <QStringList>

namespace Tape {

class Config : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString focusPolicy READ focusPolicy WRITE setFocusPolicy NOTIFY generalConfigChanged)
    Q_PROPERTY(QString decorationButtonStyle READ decorationButtonStyle WRITE setDecorationButtonStyle NOTIFY generalConfigChanged)
    Q_PROPERTY(int topBarHeight READ topBarHeight NOTIFY shellConfigChanged)
    Q_PROPERTY(int dockIconSize READ dockIconSize NOTIFY shellConfigChanged)
    Q_PROPERTY(bool dockAutoHide READ dockAutoHide NOTIFY shellConfigChanged)
    Q_PROPERTY(bool dockMagnification READ dockMagnification NOTIFY shellConfigChanged)
    Q_PROPERTY(QString wallpaperMode READ wallpaperMode NOTIFY wallpaperConfigChanged)
    Q_PROPERTY(QColor wallpaperPrimaryColor READ wallpaperPrimaryColor NOTIFY wallpaperConfigChanged)
    Q_PROPERTY(QColor wallpaperSecondaryColor READ wallpaperSecondaryColor NOTIFY wallpaperConfigChanged)
    Q_PROPERTY(QString wallpaperImagePath READ wallpaperImagePath NOTIFY wallpaperConfigChanged)
    Q_PROPERTY(bool wobblyEnabled READ wobblyEnabled NOTIFY effectsConfigChanged)
    Q_PROPERTY(bool magicLampEnabled READ magicLampEnabled NOTIFY effectsConfigChanged)
    Q_PROPERTY(bool blurEnabled READ blurEnabled NOTIFY effectsConfigChanged)
    Q_PROPERTY(int blurStrength READ blurStrength NOTIFY effectsConfigChanged)
    Q_PROPERTY(bool debugEnabled READ debugEnabled WRITE setDebugEnabled NOTIFY debugConfigChanged)

public:
    explicit Config(const QString& configFilePath = QString(), QObject* parent = nullptr);
    ~Config() override = default;

    void reload();
    void sync();

    // [General]
    QString focusPolicy() const;
    void setFocusPolicy(const QString& policy);
    QString decorationButtonStyle() const;
    void setDecorationButtonStyle(const QString& style);

    // [Render]
    QString renderLoop() const;

    // [Shell]
    int topBarHeight() const;
    int dockIconSize() const;
    bool dockAutoHide() const;
    bool dockMagnification() const;

    // [Wallpaper]
    QString wallpaperMode() const;
    QColor wallpaperPrimaryColor() const;
    QColor wallpaperSecondaryColor() const;
    QString wallpaperImagePath() const;

    // [Effects]
    bool wobblyEnabled() const;
    bool magicLampEnabled() const;
    bool blurEnabled() const;
    int blurStrength() const;

    // [InputMethod]
    bool inputMethodEnabled() const;
    QString inputMethodProgram() const;

    // [Debug]
    bool debugEnabled() const;
    void setDebugEnabled(bool enabled);
    QStringList debugCategories() const;
    bool protocolLogging() const;

Q_SIGNALS:
    void configGroupChanged(const QString& groupName);
    void generalConfigChanged();
    void shellConfigChanged();
    void wallpaperConfigChanged();
    void effectsConfigChanged();
    void debugConfigChanged();

private:
    void onConfigChanged(const KConfigGroup& group, const QByteArrayList& names);

    KSharedConfig::Ptr m_config;
    KConfigWatcher::Ptr m_watcher;
};

} // namespace Tape
