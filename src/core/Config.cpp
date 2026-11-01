// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Config.h"
#include "core/Logging.h"

#include <QFile>

namespace Tape {

Config::Config(const QString& configFilePath, QObject* parent)
    : QObject(parent)
{
    const QString targetName = configFilePath.isEmpty()
        ? QStringLiteral("taperc")
        : configFilePath;

    m_config = KSharedConfig::openConfig(targetName, KConfig::SimpleConfig);

    if (QFile::exists(QStringLiteral(TAPE_DEFAULT_CONFIG_PATH))) {
        m_config->addConfigSources({ QStringLiteral(TAPE_DEFAULT_CONFIG_PATH) });
    }

    m_watcher = KConfigWatcher::create(m_config);
    connect(m_watcher.data(), &KConfigWatcher::configChanged,
        this, &Config::onConfigChanged);

    qCInfo(lcCore, "Loaded configuration from %s", qPrintable(m_config->name()));
}

void Config::reload()
{
    m_config->reparseConfiguration();
    Q_EMIT generalConfigChanged();
    Q_EMIT shellConfigChanged();
    Q_EMIT wallpaperConfigChanged();
    Q_EMIT effectsConfigChanged();
    Q_EMIT debugConfigChanged();
}

void Config::sync()
{
    m_config->sync();
}

QString Config::focusPolicy() const
{
    return m_config->group(QStringLiteral("General"))
        .readEntry("FocusPolicy", QStringLiteral("Click"));
}

void Config::setFocusPolicy(const QString& policy)
{
    if (focusPolicy() == policy) {
        return;
    }
    m_config->group(QStringLiteral("General"))
        .writeEntry("FocusPolicy", policy, KConfigBase::Notify);
    m_config->sync();
    Q_EMIT generalConfigChanged();
}

QString Config::decorationButtonStyle() const
{
    return m_config->group(QStringLiteral("General"))
        .readEntry("DecorationButtonStyle", QStringLiteral("Breeze"));
}

void Config::setDecorationButtonStyle(const QString& style)
{
    if (decorationButtonStyle() == style) {
        return;
    }
    m_config->group(QStringLiteral("General"))
        .writeEntry("DecorationButtonStyle", style, KConfigBase::Notify);
    m_config->sync();
    Q_EMIT generalConfigChanged();
}

QString Config::renderLoop() const
{
    return m_config->group(QStringLiteral("Render"))
        .readEntry("Loop", QStringLiteral("threaded"));
}

int Config::topBarHeight() const
{
    return m_config->group(QStringLiteral("Shell"))
        .readEntry("TopBarHeight", 28);
}

int Config::dockIconSize() const
{
    return m_config->group(QStringLiteral("Shell"))
        .readEntry("DockIconSize", 48);
}

bool Config::dockAutoHide() const
{
    return m_config->group(QStringLiteral("Shell"))
        .readEntry("DockAutoHide", false);
}

bool Config::dockMagnification() const
{
    return m_config->group(QStringLiteral("Shell"))
        .readEntry("DockMagnification", true);
}

QString Config::wallpaperMode() const
{
    return m_config->group(QStringLiteral("Wallpaper"))
        .readEntry("Mode", QStringLiteral("SolidColor"));
}

QColor Config::wallpaperPrimaryColor() const
{
    return QColor(m_config->group(QStringLiteral("Wallpaper"))
                      .readEntry("PrimaryColor", QStringLiteral("#232629")));
}

QColor Config::wallpaperSecondaryColor() const
{
    return QColor(m_config->group(QStringLiteral("Wallpaper"))
                      .readEntry("SecondaryColor", QStringLiteral("#1b1e20")));
}

QString Config::wallpaperImagePath() const
{
    return m_config->group(QStringLiteral("Wallpaper"))
        .readEntry("ImagePath", QString());
}

bool Config::wobblyEnabled() const
{
    return m_config->group(QStringLiteral("Effects"))
        .readEntry("Wobbly", true);
}

bool Config::magicLampEnabled() const
{
    return m_config->group(QStringLiteral("Effects"))
        .readEntry("MagicLamp", true);
}

bool Config::blurEnabled() const
{
    return m_config->group(QStringLiteral("Effects"))
        .readEntry("Blur", true);
}

int Config::blurStrength() const
{
    return m_config->group(QStringLiteral("Effects"))
        .readEntry("BlurStrength", 6);
}

bool Config::inputMethodEnabled() const
{
    return m_config->group(QStringLiteral("InputMethod"))
        .readEntry("Enabled", true);
}

QString Config::inputMethodProgram() const
{
    return m_config->group(QStringLiteral("InputMethod"))
        .readEntry("Program", QStringLiteral("fcitx5"));
}

bool Config::debugEnabled() const
{
    return m_config->group(QStringLiteral("Debug"))
        .readEntry("Enabled", false);
}

void Config::setDebugEnabled(bool enabled)
{
    if (debugEnabled() == enabled) {
        return;
    }
    m_config->group(QStringLiteral("Debug"))
        .writeEntry("Enabled", enabled, KConfigBase::Notify);
    m_config->sync();
    Q_EMIT debugConfigChanged();
}

QStringList Config::debugCategories() const
{
    return m_config->group(QStringLiteral("Debug"))
        .readEntry("Categories", QStringList());
}

bool Config::protocolLogging() const
{
    return m_config->group(QStringLiteral("Debug"))
        .readEntry("ProtocolLogging", false);
}

void Config::onConfigChanged(const KConfigGroup& group, const QByteArrayList& names)
{
    Q_UNUSED(names)
    const QString groupName = group.name();
    qCDebug(lcCore, "KConfigWatcher notified change in group [%s]", qPrintable(groupName));
    Q_EMIT configGroupChanged(groupName);

    if (groupName == QLatin1String("General")) {
        Q_EMIT generalConfigChanged();
    } else if (groupName == QLatin1String("Shell")) {
        Q_EMIT shellConfigChanged();
    } else if (groupName == QLatin1String("Wallpaper")) {
        Q_EMIT wallpaperConfigChanged();
    } else if (groupName == QLatin1String("Effects")) {
        Q_EMIT effectsConfigChanged();
    } else if (groupName == QLatin1String("Debug")) {
        Q_EMIT debugConfigChanged();
    }
}

} // namespace Tape
