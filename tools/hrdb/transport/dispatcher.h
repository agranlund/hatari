#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <string>
#include <deque>
#include "remotecommand.h"
#include <QObject>

class QTcpSocket;
class TargetModel;

// Keeps track of messages between target and host, and matches up commands to responses,
// then passes them to the model.
class Dispatcher : public QObject
{
public:
    Dispatcher(QTcpSocket* tcpSocket, TargetModel* pTargetModel);
    virtual ~Dispatcher() override;

    uint64_t InsertFlush();

    // Request a specific memory block.
    // Allows strings so expressions can evaluate

    enum MemoryFlags
    {
        kMemFlagPhysical    = 0,
        kMemFlagLogical     = (1 << 0),
        kMemFlagSuper       = (1 << 1),
        kMemFlagUser        = (1 << 2),
        kMemFlagData        = (1 << 3),
        kMemFlagProgram     = (1 << 4),
    };

    uint64_t ReadMemory(MemorySlot slot, uint32_t address, uint32_t size, uint32_t flags = MemoryFlags::kMemFlagLogical | MemoryFlags::kMemFlagData);
    uint64_t ReadRegisters();
    uint64_t ReadInfoYm();
    uint64_t ReadBreakpoints();
    uint64_t ReadExceptionMask();
    uint64_t ReadSymbols();

    uint64_t WriteMemory(uint32_t address, const QVector<uint8_t>& data, uint32_t flags = MemoryFlags::kMemFlagLogical | MemoryFlags::kMemFlagData);

    // System control
    uint64_t ResetWarm();

    // CPU control
    uint64_t Break();
    uint64_t Run();
    uint64_t Step();
    uint64_t RunToPC(uint32_t pc);

    enum BreakpointFlags
    {
        kBpFlagNone = 0,

        kBpFlagOnce = 1 << 0,
        kBpFlagTrace = 1 << 1
    };

    uint64_t SetBreakpoint(std::string expression, uint64_t optionFlags);
    uint64_t DeleteBreakpoint(uint32_t breakpointId);

    uint64_t SetRegister(int reg, uint32_t val);
    uint64_t SetExceptionMask(uint32_t mask);
    uint64_t SetProfileEnable(bool enable);
    uint64_t SetFastForward(bool enable);
    uint64_t SendConsoleCommand(const std::string& cmd);
    uint64_t SendMemFind(const QVector<uint8_t>& valuesAndMasks, uint32_t startAddress, uint32_t endAddress);

    // Don't use this except for testing
    uint64_t DebugSendRawPacket(const char* command);

private slots:

   void connected();
   void disconnected();

   // Called by the socket class to process incoming messages
   void readyRead();

private:
    uint64_t SendCommandPacket(const char* command);
    uint64_t SendCommandShared(MemorySlot slot, std::string command);

    void ReceiveResponsePacket(const RemoteCommand& command);
    void ReceiveNotification(const RemoteNotification& notification);
    void ReceivePacket(const char* response);

    void DeletePending();

    std::deque<RemoteCommand*>      m_sentCommands;
    QTcpSocket*                     m_pTcpSocket;
    TargetModel*                    m_pTargetModel;

    std::string                     m_active_resp;
    uint64_t                        m_responseUid;

    /* If true, drop incoming packets since they are assumed to be
     * from a previous connection. */
    bool                            m_portConnected;
    bool                            m_waitingConnectionAck;
};

#endif // DISPATCHER_H
