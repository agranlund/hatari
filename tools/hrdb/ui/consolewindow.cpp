#include "consolewindow.h"

#include <iostream>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QSettings>
#include <QTemporaryFile>
#include <QTextStream>
#include <QTextEdit>
#include <QDebug>

#include "../transport/dispatcher.h"
#include "../models/targetmodel.h"
#include "../models/session.h"
#include "quicklayout.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ConsoleWindow::ConsoleWindow(QWidget *parent, Session* pSession) :
    QDockWidget(parent),
    m_pSession(pSession),
    m_pTargetModel(pSession->m_pTargetModel),
    m_pDispatcher(pSession->m_pDispatcher)
{
    this->setWindowTitle("Console");
    setObjectName("Console");

    m_pLineEdit = new QLineEdit(this);
    m_pTextArea = new QTextEdit(this);
    m_pTextArea->setReadOnly(true);

    // Layouts
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    QHBoxLayout* pTopLayout = new QHBoxLayout;
    auto pMainRegion = new QWidget(this);   // whole panel
    auto pTopRegion = new QWidget(this);      // top buttons/edits

    SetMargins(pTopLayout);
    pTopLayout->addWidget(m_pLineEdit);

    pMainLayout->setSpacing(0);
    pMainLayout->setContentsMargins(0,0,0,0);
    pMainLayout->addWidget(pTopRegion);
    pMainLayout->addWidget(m_pTextArea);

    pTopRegion->setLayout(pTopLayout);
    pMainRegion->setLayout(pMainLayout);
    setWidget(pMainRegion);

    loadSettings();

    connect(m_pTargetModel,  &TargetModel::connectChangedSignal, this, &ConsoleWindow::connectChanged);
    connect(m_pTargetModel,  &TargetModel::logReceivedSignal,    this, &ConsoleWindow::logMessage);
    connect(m_pSession,      &Session::settingsChanged,          this, &ConsoleWindow::settingsChanged);
    // Connect text entry
    connect(m_pLineEdit,     &QLineEdit::returnPressed,          this, &ConsoleWindow::textEditChanged);

    // Refresh enable state
    connectChanged();

    // Refresh font
    settingsChanged();
}

ConsoleWindow::~ConsoleWindow()
{
}

void ConsoleWindow::keyFocus()
{
    activateWindow();
    m_pLineEdit->setFocus();
}

void ConsoleWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup("Console");

    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();
}

void ConsoleWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("Console");

    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void ConsoleWindow::logMessage(const char* pStr)
{
    QString data(pStr);
    m_pTextArea->moveCursor(QTextCursor::End);
    m_pTextArea->insertPlainText(data);
    m_pTextArea->moveCursor(QTextCursor::End);

}

void ConsoleWindow::connectChanged()
{
    bool enable = m_pTargetModel->IsConnected();
    m_pLineEdit->setEnabled(enable);
}

void ConsoleWindow::settingsChanged()
{
    m_pTextArea->setFont(m_pSession->GetSettings().m_font);
}

void ConsoleWindow::textEditChanged()
{
    if (m_pTargetModel->IsConnected() && !m_pTargetModel->IsRunning())
    {
        m_pTextArea->moveCursor(QTextCursor::End);
        m_pTextArea->append(QString(">>") + m_pLineEdit->text() + QString("\n"));
        m_pTextArea->moveCursor(QTextCursor::End);
        m_pDispatcher->SendConsoleCommand(m_pLineEdit->text().toStdString());
    }
    m_pLineEdit->clear();
}
