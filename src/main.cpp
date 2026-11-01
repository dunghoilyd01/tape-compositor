// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Application.h"
#include "core/CommandLine.h"

#include <QGuiApplication>
#include <cstdio>

int main(int argc, char* argv[])
{
    if (Tape::CommandLine::hasHeadlessFlag(argc, argv)) {
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    }

    QCoreApplication::setApplicationName(QStringLiteral("tape"));
    QCoreApplication::setOrganizationName(QStringLiteral("Tape"));
    QCoreApplication::setApplicationVersion(QStringLiteral(TAPE_VERSION_STRING));

    QGuiApplication guiApp(argc, argv);
    QGuiApplication::setQuitOnLastWindowClosed(false);

    const Tape::CommandLineOptions options = Tape::CommandLine::parse(QCoreApplication::arguments());
    if (!options.parseOk) {
        std::fprintf(stderr, "tape: %s\n", qPrintable(options.errorMessage));
        return 1;
    }

    Tape::Application tapeApp(options);
    if (!tapeApp.initialize()) {
        return 1;
    }

    return guiApp.exec();
}
