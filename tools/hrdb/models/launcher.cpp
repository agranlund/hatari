#include "launcher.h"
#include <QTemporaryFile>
#include <QTextStream>
#include <QProcess>
#include <QSettings>
#include <QFileSystemWatcher>
#include <QObject>

#include "session.h"
#include "filewatcher.h"

void LaunchSettings::loadSettings(QSettings& settings)
{
    // Save launcher settings
    settings.beginGroup("LaunchSettings");
    m_hatariFilename = settings.value("exe", QVariant("hatari")).toString();
    m_argsTxt = settings.value("args", QVariant("")).toString();
    m_prgFilename = settings.value("prg", QVariant("")).toString();
    m_workingDirectory = settings.value("workingDirectory", QVariant("")).toString();
    m_watcherFiles = settings.value("watcherFiles", QVariant("")).toString();
    m_watcherActive = settings.value("watcherActive", QVariant("false")).toBool();
    m_breakMode = settings.value("breakMode", QVariant("0")).toInt();
    m_fastLaunch = settings.value("fastLaunch", QVariant("false")).toBool();
    m_breakPointTxt = settings.value("breakPointTxt", QVariant("")).toString();
    settings.endGroup();
}

void LaunchSettings::saveSettings(QSettings &settings) const
{
    settings.beginGroup("LaunchSettings");
    settings.setValue("exe", m_hatariFilename);
    settings.setValue("args", m_argsTxt);
    settings.setValue("prg", m_prgFilename);
    settings.setValue("workingDirectory", m_workingDirectory);
    settings.setValue("watcherFiles", m_watcherFiles);
    settings.setValue("watcherActive", m_watcherActive);
    settings.setValue("breakMode", m_breakMode);
    settings.setValue("fastLaunch", m_fastLaunch);
    settings.setValue("breakPointTxt", m_breakPointTxt);
    settings.endGroup();
}

bool LaunchHatari(const LaunchSettings& settings, const Session* pSession)
{
    // Create a copy of the args that we can adjust
    QStringList args;
    QString otherArgsText = settings.m_argsTxt;
    otherArgsText = otherArgsText.trimmed();
    if (otherArgsText.size() != 0)
        args = otherArgsText.split(" ");

    if (settings.m_watcherActive)
    {
        FileWatcher* pFileWatcher=((Session*)pSession)->createFileWatcherInstance();
        if (pFileWatcher)
        {
            pFileWatcher->clear(); //remove all watched files
            if(settings.m_watcherFiles.length()>0)
                pFileWatcher->addPaths(settings.m_watcherFiles.split(","));
            else
                pFileWatcher->addPath(settings.m_prgFilename);
        }
    }

    // First make a temp file for breakpoints etc

    {
        // Breakpoint script file
        {
            QString tmpContents;
            QTextStream ref(&tmpContents);

            if (settings.m_fastLaunch)
            {
                ref << "setopt --fast-forward 0\r\n";   // in bp file
                args.push_front("1");
                args.push_front("--fast-forward");      // in startup args
            }

            ref << "symbols prg\r\n";
            if (settings.m_breakMode == LaunchSettings::kProgramBreakpoint)
                ref << "b " << settings.m_breakPointTxt << ":once\r\n";

            // Create the temp file
            QTemporaryFile& tmp(*pSession->m_pProgramStartScript);
            if (tmp.exists())
                tmp.remove();
            if (!tmp.open())
                return false;

            tmp.setTextModeEnabled(true);
            tmp.write(tmpContents.toUtf8());
            tmp.close();
        }

        // Startup script file
        {
            QString tmpContents;
            QTextStream ref(&tmpContents);
            QString progStartFilename = pSession->m_pProgramStartScript->fileName();

            // Generate some commands for
            // Break at boot/start commands
            if (settings.m_breakMode == LaunchSettings::BreakMode::kBoot)
                ref << "b pc ! 0 : once\r\n";     // don't run the breakpoint file yet

            if (settings.m_breakMode == LaunchSettings::BreakMode::kProgStart)
            {
                // Break at program start and run the program start script
                ref << "b pc=TEXT && pc<$e00000 :once :file " << progStartFilename << "\r\n";
            }
            else if (settings.m_fastLaunch ||
                     settings.m_breakMode == LaunchSettings::BreakMode::kProgramBreakpoint)
            {
               // run bp file but don't stop
                ref << "b pc=TEXT && pc<$e00000 :trace :once :file " << progStartFilename << "\r\n";
            }

            // Create the temp file
            // In theory we need to be careful about reuse?
            QTemporaryFile& tmp(*pSession->m_pStartupFile);
            if (tmp.exists())
                tmp.remove();

            if (!tmp.open())
                return false;

            tmp.setTextModeEnabled(true);
            tmp.write(tmpContents.toUtf8());
            tmp.close();

            // Prepend the "--parse N" part (backwards!)
            args.push_front(tmp.fileName());
            args.push_front("--parse");
        }
    }

    // Executable goes as last arg
    args.push_back(settings.m_prgFilename);

    // Actually launch the program
    QProcess proc;
    proc.setProgram(settings.m_hatariFilename);
    proc.setArguments(args);

    // Redirect outputs to NULL so that Hatari's own spew doesn't cause lockups
    // if hrdb is killed and restarted (temp file contention?)
    proc.setStandardOutputFile(QProcess::nullDevice());
    proc.setStandardErrorFile(QProcess::nullDevice());
    proc.setWorkingDirectory(settings.m_workingDirectory);
    return proc.startDetached();
}
