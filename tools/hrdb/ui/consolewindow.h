#ifndef CONSOLEWINDOW_H
#define CONSOLEWINDOW_H

#include <QDockWidget>
#include <QFile>
#include <QTextStream>
#include "../models/memory.h"

class TargetModel;
class Dispatcher;
class Session;
class QLabel;
class QLineEdit;
class QTextEdit;
class QFileSystemWatcher;

class ConsoleWindow : public QDockWidget
{
    Q_OBJECT
public:
    ConsoleWindow(QWidget *parent, Session* pSession);
    virtual ~ConsoleWindow();

    // Grab focus and point to the main widget
    void keyFocus();

    void loadSettings();
    void saveSettings();

private:
    void connectChanged();
    void settingsChanged();
    void textEditChanged();
    void logMessage(const char* pStr);

    void deleteWatcher();
    QLineEdit*          m_pLineEdit;
    QTextEdit*          m_pTextArea;

    Session*            m_pSession;
    TargetModel*        m_pTargetModel;
    Dispatcher*         m_pDispatcher;
};

#endif // CONSOLEWINDOW_H
