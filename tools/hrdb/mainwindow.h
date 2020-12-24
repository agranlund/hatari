#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*
	Main GUI Window.

	Currently everything is controlled and routed through here.
*/

#include <QMainWindow>
#include "targetmodel.h"
#include "disassembler.h"

class QPushButton;
class QLabel;
class QTcpSocket;
class QTextEdit;
class QActionGroup;
class QComboBox;

class Dispatcher;
class TargetModel;

class DisasmWidget;
class MemoryViewWidget;

class ExceptionDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    virtual void closeEvent(QCloseEvent *event);

private slots:
    void connectChangedSlot();
	void startStopChangedSlot();
    void registersChangedSlot(uint64_t commandId);
    void memoryChangedSlot(int slot, uint64_t commandId);
    void symbolTableChangedSlot(uint64_t commandId);
    void startStopDelayedSlot(int running);

    void startStopClicked();
    void singleStepClicked();
    void nextClicked();
    void runToClicked();
    void updateWindowMenu();
    void menuConnect();
    void menuDisconnect();
    void menuDisasmWindow0();
    void menuDisasmWindow1();
    void menuMemoryWindow0();
    void menuMemoryWindow1();
    void about();
    void aboutQt();
private:
    // Network
    void Connect();
    void Disconnect();

    void ExceptionsDialog();

	// Populaters
	void PopulateRegisters();
    QString FindSymbol(uint32_t addr);
    void PopulateRunningSquare();
    void updateButtonEnable();

    // Our UI widgets
    QPushButton*	m_pStartStopButton;
    QPushButton*	m_pStepIntoButton;
    QPushButton*	m_pStepOverButton;
    QPushButton*	m_pRunToButton;
    QComboBox*      m_pRunToCombo;
	QTextEdit*		m_pRegistersTextEdit;
    QWidget*        m_pRunningSquare;

    // Dialogs
    ExceptionDialog*    m_pExceptionDialog;

    // Docking windows
    DisasmWidget*       m_pDisasmWidget0;
    DisasmWidget*       m_pDisasmWidget1;
    MemoryViewWidget*   m_pMemoryViewWidget0;
    MemoryViewWidget*   m_pMemoryViewWidget1;

    // Low-level data
	QTcpSocket* 	tcpSocket;
    Dispatcher*		m_pDispatcher;
	TargetModel*	m_pTargetModel;

    // Shown data
    Registers                      m_prevRegs;
    Disassembler::disassembly      m_disasm;

    // Menus
    void createActions();
    void createMenus();
    void toggleVis(QWidget *pWidget);
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *windowMenu;
    QMenu *helpMenu;

    QAction *connectAct;
    QAction *disconnectAct;
    QAction *exitAct;

    QAction *exceptionsAct;

    QAction *disasmWindowAct0;
    QAction *disasmWindowAct1;
    QAction *memoryWindowAct0;
    QAction *memoryWindowAct1;
    QAction *aboutAct;
    QAction *aboutQtAct;
    void readSettings();
    void writeSettings();
};
#endif // MAINWINDOW_H
