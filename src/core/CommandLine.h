// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QSize>
#include <QString>
#include <QStringList>

namespace Tape {

struct OutputSpec {
    QSize size { 1280, 800 };
    double scale = 1.0;
};

struct CommandLineOptions {
    bool nested = false;
    bool headless = false;
    int outputCount = 1;
    OutputSpec outputSpec;
    bool debug = false;
    QStringList debugCategories;
    bool protocolLogging = false;
    QString configPath;
    QString socketName;
    QString startupCommand;
    bool parseOk = true;
    QString errorMessage;
};

class CommandLine {
public:
    static bool hasHeadlessFlag(int argc, char* argv[]);
    static CommandLineOptions parse(const QStringList& arguments);
    static OutputSpec parseOutputSpec(const QString& spec, bool* ok = nullptr);
};

} // namespace Tape
