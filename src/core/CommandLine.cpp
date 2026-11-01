// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/CommandLine.h"

#include <QCommandLineOption>
#include <QCommandLineParser>

#include <cstring>

namespace Tape {

bool CommandLine::hasHeadlessFlag(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--headless") == 0) {
            return true;
        }
    }
    return false;
}

OutputSpec CommandLine::parseOutputSpec(const QString& spec, bool* ok)
{
    OutputSpec result;
    if (ok != nullptr) {
        *ok = false;
    }

    const QString trimmed = spec.trimmed();
    if (trimmed.isEmpty()) {
        return result;
    }

    const QStringList scaleParts = trimmed.split(QLatin1Char('@'));
    if (scaleParts.size() > 2) {
        return result;
    }

    const QStringList dimParts = scaleParts[0].toLower().split(QLatin1Char('x'));
    if (dimParts.size() != 2) {
        return result;
    }

    bool wOk = false;
    bool hOk = false;
    const int width = dimParts[0].toInt(&wOk);
    const int height = dimParts[1].toInt(&hOk);
    if (!wOk || !hOk || width < 320 || height < 240 || width > 16384 || height > 16384) {
        return result;
    }

    double scale = 1.0;
    if (scaleParts.size() == 2) {
        bool sOk = false;
        scale = scaleParts[1].toDouble(&sOk);
        if (!sOk || scale < 0.5 || scale > 4.0) {
            return result;
        }
    }

    result.size = QSize(width, height);
    result.scale = scale;
    if (ok != nullptr) {
        *ok = true;
    }
    return result;
}

CommandLineOptions CommandLine::parse(const QStringList& arguments)
{
    CommandLineOptions opts;

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Tape Compositor — Lightweight KDE-themed Stacking Wayland Compositor"));
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption nestedOpt(
        QStringLiteral("nested"),
        QStringLiteral("Run nested inside an existing Wayland or X11 session."));
    const QCommandLineOption headlessOpt(
        QStringLiteral("headless"),
        QStringLiteral("Run in offscreen/headless mode for automated tests."));
    const QCommandLineOption outputsOpt(
        QStringLiteral("outputs"),
        QStringLiteral("Number of outputs in nested/headless mode (default: 1)."),
        QStringLiteral("count"),
        QStringLiteral("1"));
    const QCommandLineOption outputSizeOpt(
        QStringLiteral("output-size"),
        QStringLiteral("Output geometry WxH[@scale] (default: 1280x800@1.0)."),
        QStringLiteral("WxH@scale"),
        QStringLiteral("1280x800"));
    const QCommandLineOption debugOpt(
        QStringLiteral("debug"),
        QStringLiteral("Enable runtime debug logging at startup."));
    const QCommandLineOption debugCatsOpt(
        QStringLiteral("debug-categories"),
        QStringLiteral("Comma-separated debug categories (e.g. core,wm,drm)."),
        QStringLiteral("categories"));
    const QCommandLineOption protoLogOpt(
        QStringLiteral("protocol-log"),
        QStringLiteral("Enable Wayland server protocol logging."));
    const QCommandLineOption configOpt(
        QStringLiteral("config"),
        QStringLiteral("Path to configuration file (default: ~/.config/taperc)."),
        QStringLiteral("path"));
    const QCommandLineOption socketOpt(
        QStringList { QStringLiteral("s"), QStringLiteral("socket") },
        QStringLiteral("Wayland socket name (default: auto-selected by libwayland)."),
        QStringLiteral("name"));
    const QCommandLineOption commandOpt(
        QStringList { QStringLiteral("c"), QStringLiteral("command") },
        QStringLiteral("Command to execute after the Wayland socket is ready."),
        QStringLiteral("cmd"));

    parser.addOption(nestedOpt);
    parser.addOption(headlessOpt);
    parser.addOption(outputsOpt);
    parser.addOption(outputSizeOpt);
    parser.addOption(debugOpt);
    parser.addOption(debugCatsOpt);
    parser.addOption(protoLogOpt);
    parser.addOption(configOpt);
    parser.addOption(socketOpt);
    parser.addOption(commandOpt);

    if (!parser.parse(arguments)) {
        opts.parseOk = false;
        opts.errorMessage = parser.errorText();
        return opts;
    }

    if (parser.isSet(QStringLiteral("help"))) {
        parser.showHelp(0);
    }
    if (parser.isSet(QStringLiteral("version"))) {
        parser.showVersion();
    }

    opts.headless = parser.isSet(headlessOpt);
    opts.nested = parser.isSet(nestedOpt)
        || (!opts.headless
            && (!qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")
                || !qEnvironmentVariableIsEmpty("DISPLAY")));

    bool countOk = false;
    const int count = parser.value(outputsOpt).toInt(&countOk);
    if (!countOk || count < 1 || count > 8) {
        opts.parseOk = false;
        opts.errorMessage = QStringLiteral("Invalid --outputs count (must be 1..8)");
        return opts;
    }
    opts.outputCount = count;

    bool specOk = false;
    opts.outputSpec = parseOutputSpec(parser.value(outputSizeOpt), &specOk);
    if (!specOk) {
        opts.parseOk = false;
        opts.errorMessage = QStringLiteral("Invalid --output-size format (expected WxH or WxH@scale)");
        return opts;
    }

    opts.debug = parser.isSet(debugOpt);
    if (parser.isSet(debugCatsOpt)) {
        opts.debugCategories = parser.value(debugCatsOpt).split(QLatin1Char(','), Qt::SkipEmptyParts);
        opts.debug = true;
    }
    opts.protocolLogging = parser.isSet(protoLogOpt);
    opts.configPath = parser.value(configOpt);
    opts.socketName = parser.value(socketOpt);
    opts.startupCommand = parser.value(commandOpt);

    return opts;
}

} // namespace Tape
