#ifndef TARGET_MODEL_H
#define TARGET_MODEL_H

#include <stdint.h>
#include <QObject>
#include <QVector>

#include "transport/remotecommand.h"
#include "breakpoint.h"
#include "memory.h"
#include "symboltable.h"
#include "registers.h"
#include "exceptionmask.h"
#include "../hardware/hardware_st.h"
#include "../hopper/decode.h"

class QTimer;
class ProfileData;
struct ProfileDelta;

class TargetChangedFlags
{
public:
    enum ChangedState
    {
        kPC,
        kRegs,
        kBreakpoints,
        kSymbolTable,
        kExceptionMask,
        kOtherMemory,        // e.g. a control set memory elsewhere
        kChangedStateCount
    };

    void Clear();
    void SetChanged(ChangedState ch) { m_changed[ch] = true; }
    void SetMemoryChanged(MemorySlot slot) { m_memChanged[slot] = true; }

    bool    m_changed[kChangedStateCount];
    bool    m_memChanged[kMemorySlotCount];
};

/*
    Simple container for YM register state
*/
class YmState
{
public:
    YmState();
    static const int kNumRegs = 16;
    void Clear();
    uint8_t m_regs[kNumRegs];
};

/* Simple container for search results
 *
*/
class SearchResults
{
public:
    QVector<uint32_t> addresses;
};

/*
    Core central data model reflecting the state of the target.
*/
class TargetModel : public QObject
{
	Q_OBJECT
public:
    enum CpuLevel
    {
        kCpuLevel68000 = 0,
        kCpuLevel68010 = 1,
        kCpuLevel68020 = 2,
        kCpuLevel68030 = 3,
        kCpuLevel68040 = 4,
        kCpuLevel68060 = 6,
    };

    TargetModel();
    virtual ~TargetModel();

    // These are called by the Dispatcher when notifications/events arrive
    void SetConnected(int running);
    void SetStatus(bool running, uint32_t pc, bool ffwd);
    void SetConfig(uint32_t machineType, uint32_t cpuLevel, uint32_t stRamSize);

    // These are called by the Dispatcher when responses arrive
    // emits registersChangedSignal()
    void SetRegisters(const Registers& regs, uint64_t commandId);

    // emits memoryChangedSignal()
    void SetMemory(MemorySlot slot, const Memory* pMem, uint64_t commandId);

    // emits breakpointsChangedSignal()
    void SetBreakpoints(const Breakpoints& bps, uint64_t commandId);

    // Set Hatari's subtable of symbols
    // emits symbolTableChangedSignal()
    void SetSymbolTable(const SymbolSubTable& syms, uint64_t commandId);

    // emits exceptionMaskChanged()
    void SetExceptionMask(const ExceptionMask& mask);

    // Sets YM registers/internals
    // emits ymChangedSignal()
    void SetYm(const YmState& state);

    // Called when a memset command is known to have changed a memory region.
    // emits otherMemoryChangedSignal()
    void NotifyMemoryChanged(uint32_t address, uint32_t size);

    // emits searchResultsChangedSignal()
    void SetSearchResults(uint64_t commmandId, const SearchResults& results);

    // The following 2 commands are processed as a batch
    // Update profiling data. Does not emit signal
    void AddProfileDelta(const ProfileDelta& delta);
    // emits profileChangedSignal()
    void ProfileDeltaComplete(int enabled);

    // (Called from UI)
    // emits profileChangedSignal()
    void ProfileReset();

    // User-added console command. Anything can happen!
    void ConsoleCommand();

    void LogMessage(const char* pStr);

    // User-inserted dummy command to signal e.g. end of a series of updates
    // emits flushSignal()
    void Flush(uint64_t commmandId);

    // =========================== DATA ACCESS =================================
	// NOTE: all these return copies to avoid data contention
    MACHINETYPE	GetMachineType() const { return m_machineType; }
    int GetCpuLevel() const { return m_cpuLevel; }

    uint32_t GetAddressMask() const { return m_cpuLevel > 0 ? 0xffffffff : 0x00ffffff; }
    uint32_t GetVbr() const { return m_cpuLevel > 0 ? GetRegs().Get(Registers::VBR) : 0; }

    // Get RAM size in bytes
    uint32_t GetSTRamSize() const { return m_stRamSize; }

    int IsConnected() const { return m_bConnected; }
    int IsRunning() const { return m_bRunning; }
    int IsFastForward() const { return m_ffwd; }
    int IsProfileEnabled() const { return m_bProfileEnabled; }

    // This is the PC from start/stop notifications, so it's not valid when
    // running
    uint32_t GetStartStopPC() const { return m_startStopPc; }
	Registers GetRegs() const { return m_regs; }
    const Memory* GetMemory(MemorySlot slot) const
    {
        return m_pMemory[slot];
    }
    const Breakpoints& GetBreakpoints() const { return m_breakpoints; }
    const SymbolTable& GetSymbolTable() const { return m_symbolTable; }
    const SearchResults& GetSearchResults() const { return m_searchResults; }
    const ExceptionMask& GetExceptionMask() const { return m_exceptionMask; }
    YmState GetYm() const { return m_ymState; }

    // Profiling access
    void GetProfileData(uint32_t addr, uint32_t& count, uint32_t& cycles) const;
    const ProfileData& GetRawProfileData() const;

    // CPU info for disassembly
    const decode_settings& GetDisasmSettings() const;

public slots:

signals:
    // connect/disconnect change
    void connectChangedSignal();

    // When start/stop status is changed
    void startStopChangedSignal();

    void startStopChangedSignalDelayed(int running);

    // When running, and the periodic refresh timer signals
    void runningRefreshTimerSignal();

    // When a user-inserted flush is the next command
    void flushSignal(const TargetChangedFlags& flags, uint64_t uid);

	// When new CPU registers are changed
    void registersChangedSignal(uint64_t commandId);

    // When a block of fetched memory is changed/updated
    void memoryChangedSignal(int memorySlot, uint64_t commandId);

    // When breakpoints data is updated
    void breakpointsChangedSignal(uint64_t commandId);

    // When symbol table is updated
    void symbolTableChangedSignal(uint64_t commandId);

    // When search results returned. Use GetSearchResults()
    void searchResultsChangedSignal(uint64_t commandId);

    // When exception mask updated
    void exceptionMaskChanged();

    // When new YM state updated
    void ymChangedSignal();

    // Something edited memory
    void otherMemoryChangedSignal(uint32_t address, uint32_t size);

    // Profile data changed
    void profileChangedSignal();

    // Log message received
    void logReceivedSignal(const char* pStr);

private slots:

    // Called shortly after stop notification received
    void delayedTimer();
    void runningRefreshTimerSlot();

private:
    TargetChangedFlags  m_changedFlags;

    MACHINETYPE     m_machineType;	// Hatari MACHINETYPE enum
    uint32_t        m_cpuLevel;		// CPU 0=000, 1=010, 2=020, 3=030, 4=040, 5=060
    uint32_t        m_stRamSize;    // Size of available ST Ram
    decode_settings m_decodeSettings;

    int             m_bConnected;   // 0 == disconnected, 1 == connected
    int             m_bRunning;		// 0 == stopped, 1 == running
    int             m_bProfileEnabled; // 0 == off, 1 == collecting
    uint32_t        m_startStopPc;	// PC register (for next instruction)
    int             m_ffwd;         // 0 == normal, 1 == fast forward mode

    Registers       m_regs;			// Current register values
    Breakpoints     m_breakpoints;  // Current breakpoint list
    SymbolTable     m_symbolTable;
    ExceptionMask   m_exceptionMask;
    YmState         m_ymState;
    ProfileData*    m_pProfileData;

    SearchResults   m_searchResults;

    // Actual current memory contents
    const Memory*   m_pMemory[MemorySlot::kMemorySlotCount];

    // Timer running to trigger events after CPU has stopped for a while
    // (e.g. Graphics Inspector refresh)
    QTimer*         m_pDelayedUpdateTimer;

    // Timer running to support refresh while running
    QTimer*         m_pRunningRefreshTimer;
};

// Helper functions to check broad machine types
extern bool IsMachineST(MACHINETYPE type);
extern bool IsMachineSTE(MACHINETYPE type);

#endif // TARGET_MODEL_H
