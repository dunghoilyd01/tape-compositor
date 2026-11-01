// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/CommandLine.h"
#include "core/Compositor.h"
#include "core/Config.h"

#include <QObject>
#include <QSocketNotifier>
#include <memory>

namespace Tape {

class Application : public QObject {
    Q_OBJECT

public:
    explicit Application(const CommandLineOptions& options, QObject* parent = nullptr);
    ~Application() override;

    bool initialize(const QString& customLogDir = QString());

    Compositor* compositor() const { return m_compositor.get(); }
    Config* config() const { return m_config.get(); }
    const CommandLineOptions& options() const { return m_options; }

private Q_SLOTS:
    void handleUnixSignal();

private:
    static void unixSignalHandler(int sig);
    void setupUnixSignalHandlers();
    void launchStartupCommand();

    static int s_sigFd[2];

    CommandLineOptions m_options;
    std::unique_ptr<Config> m_config;
    std::unique_ptr<Compositor> m_compositor;
    QSocketNotifier* m_sigNotifier = nullptr;
};

} // namespace Tape
