/*
  Hatari - memorySnapShot.c

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.

  Memory Snapshot

  This handles the saving/restoring of the emulator's state so any game or
  application can be saved and restored at any time. This is quite complicated
  as we need to store all STRam, all chip states, all emulation variables and
  then things get really complicated as we need to restore file handles
  and such like.
  To help keep things simple each file has one function which is used to
  save/restore all variables that are local to it. We use one function to
  reduce redundancy and the function 'MemorySnapShot_Store' decides if it
  should save or restore the data.
*/
const char MemorySnapShot_fileid[] = "Hatari memorySnapShot.c : " __DATE__ " " __TIME__;

#include <SDL_types.h>
#include <errno.h>

#include "main.h"
#include "blitter.h"
#include "configuration.h"
#include "debugui.h"
#include "dmaSnd.h"
#include "fdc.h"
#include "file.h"
#include "floppy.h"
#include "gemdos.h"
#include "ikbd.h"
#include "cycInt.h"
#include "cycles.h"
#include "ioMem.h"
#include "log.h"
#include "m68000.h"
#include "memorySnapShot.h"
#include "mfp.h"
#include "psg.h"
#include "reset.h"
#include "sound.h"
#include "str.h"
#include "stMemory.h"
#include "tos.h"
#include "screen.h"
#include "video.h"
#include "falcon/dsp.h"
#include "falcon/crossbar.h"
#include "statusbar.h"


#define VERSION_STRING      "1.4.0"   /* Version number of compatible memory snapshots - Always 6 bytes (inc' NULL) */
#define VERSION_STRING_SIZE    6      /* Size of above (inc' NULL) */


#define COMPRESS_MEMORYSNAPSHOT       /* Compress snapshots to reduce disk space used */

#ifdef COMPRESS_MEMORYSNAPSHOT

#include <zlib.h>
typedef gzFile MSS_File;

#else

typedef FILE* MSS_File;

#endif


static MSS_File CaptureFile;
static bool bCaptureSave, bCaptureError;


/*-----------------------------------------------------------------------*/
/**
 * Open file.
 */
static MSS_File MemorySnapShot_fopen(const char *pszFileName, const char *pszMode)
{
#ifdef COMPRESS_MEMORYSNAPSHOT
	return gzopen(pszFileName, pszMode);
#else
	return fopen(pszFileName, pszMode);
#endif
}


/*-----------------------------------------------------------------------*/
/**
 * Close file.
 */
static void MemorySnapShot_fclose(MSS_File fhndl)
{
#ifdef COMPRESS_MEMORYSNAPSHOT
	gzclose(fhndl);
#else
	fclose(fhndl);
#endif
}


/*-----------------------------------------------------------------------*/
/**
 * Read from file.
 */
static int MemorySnapShot_fread(MSS_File fhndl, char *buf, int len)
{
#ifdef COMPRESS_MEMORYSNAPSHOT
	return gzread(fhndl, buf, len);
#else
	return fread(buf, 1, len, fhndl);
#endif
}


/*-----------------------------------------------------------------------*/
/**
 * Write data to file.
 */
static int MemorySnapShot_fwrite(MSS_File fhndl, const char *buf, int len)
{
#ifdef COMPRESS_MEMORYSNAPSHOT
	return gzwrite(fhndl, buf, len);
#else
	return fwrite(buf, 1, len, fhndl);
#endif
}


/*-----------------------------------------------------------------------*/
/**
 * Open/Create snapshot file, and set flag so 'MemorySnapShot_Store' knows
 * how to handle data.
 */
static bool MemorySnapShot_OpenFile(const char *pszFileName, bool bSave)
{
	char VersionString[VERSION_STRING_SIZE];

	/* Set error */
	bCaptureError = false;

	/* Open file, set flag so 'MemorySnapShot_Store' can load to/save from file */
	if (bSave)
	{
		/* Save */
		CaptureFile = MemorySnapShot_fopen(pszFileName, "wb");
		if (!CaptureFile)
		{
			fprintf(stderr, "Failed to open save file '%s': %s\n",
			        pszFileName, strerror(errno));
			bCaptureError = true;
			return false;
		}
		bCaptureSave = true;
		/* Store version string */
		strcpy(VersionString, VERSION_STRING);
		MemorySnapShot_Store(VersionString, VERSION_STRING_SIZE);
	}
	else
	{
		/* Restore */
		CaptureFile = MemorySnapShot_fopen(pszFileName, "rb");
		if (!CaptureFile)
		{
			fprintf(stderr, "Failed to open file '%s': %s\n",
			        pszFileName, strerror(errno));
			bCaptureError = true;
			return false;
		}
		bCaptureSave = false;
		/* Restore version string */
		MemorySnapShot_Store(VersionString, VERSION_STRING_SIZE);
		/* Does match current version? */
		if (strcasecmp(VersionString, VERSION_STRING))
		{
			/* No, inform user and error */
			Log_AlertDlg(LOG_ERROR, "Unable to Restore Memory State.\nFile is "
			                       "only compatible with Hatari v%s", VersionString);
			bCaptureError = true;
			return false;
		}
	}

	/* All OK */
	return true;
}


/*-----------------------------------------------------------------------*/
/**
 * Close snapshot file.
 */
static void MemorySnapShot_CloseFile(void)
{
	MemorySnapShot_fclose(CaptureFile);
}


/*-----------------------------------------------------------------------*/
/**
 * Save/Restore data to/from file.
 */
void MemorySnapShot_Store(void *pData, int Size)
{
	long nBytes;

	/* Check no file errors */
	if (CaptureFile != NULL)
	{
		/* Saving or Restoring? */
		if (bCaptureSave)
			nBytes = MemorySnapShot_fwrite(CaptureFile, (char *)pData, Size);
		else
			nBytes = MemorySnapShot_fread(CaptureFile, (char *)pData, Size);

		/* Did save OK? */
		if (nBytes != Size)
			bCaptureError = true;
	}
}


/*-----------------------------------------------------------------------*/
/**
 * Save 'snapshot' of memory/chips/emulation variables
 */
void MemorySnapShot_Capture(const char *pszFileName, bool bConfirm)
{
	/* Set to 'saving' */
	if (MemorySnapShot_OpenFile(pszFileName, true))
	{
		/* Capture each files details */
		Configuration_MemorySnapShot_Capture(true);
		TOS_MemorySnapShot_Capture(true);
		STMemory_MemorySnapShot_Capture(true);
		FDC_MemorySnapShot_Capture(true);
		Floppy_MemorySnapShot_Capture(true);
		GemDOS_MemorySnapShot_Capture(true);
		IKBD_MemorySnapShot_Capture(true);
		CycInt_MemorySnapShot_Capture(true);
		Cycles_MemorySnapShot_Capture(true);
		M68000_MemorySnapShot_Capture(true);
		MFP_MemorySnapShot_Capture(true);
		PSG_MemorySnapShot_Capture(true);
		Sound_MemorySnapShot_Capture(true);
		Video_MemorySnapShot_Capture(true);
		Blitter_MemorySnapShot_Capture(true);
		DmaSnd_MemorySnapShot_Capture(true);
		Crossbar_MemorySnapShot_Capture(true);
		DSP_MemorySnapShot_Capture(true);
		DebugUI_MemorySnapShot_Capture(pszFileName, true);
		/* And close */
		MemorySnapShot_CloseFile();
	}

	/* Did error */
	if (bCaptureError)
		Log_AlertDlg(LOG_ERROR, "Unable to save memory state to file.");
	else if (bConfirm)
		Log_AlertDlg(LOG_INFO, "Memory state file saved.");
}


/*-----------------------------------------------------------------------*/
/**
 * Restore 'snapshot' of memory/chips/emulation variables
 */
void MemorySnapShot_Restore(const char *pszFileName, bool bConfirm)
{
	/* Set to 'restore' */
	if (MemorySnapShot_OpenFile(pszFileName, false))
	{
		Configuration_MemorySnapShot_Capture(false);
		TOS_MemorySnapShot_Capture(false);

		/* Reset emulator to get things running */
		IoMem_UnInit();  IoMem_Init();
		Reset_Cold();

		/* Capture each files details */
		STMemory_MemorySnapShot_Capture(false);
		FDC_MemorySnapShot_Capture(false);
		Floppy_MemorySnapShot_Capture(false);
		GemDOS_MemorySnapShot_Capture(false);
		IKBD_MemorySnapShot_Capture(false);
		CycInt_MemorySnapShot_Capture(false);
		Cycles_MemorySnapShot_Capture(false);
		M68000_MemorySnapShot_Capture(false);
		MFP_MemorySnapShot_Capture(false);
		PSG_MemorySnapShot_Capture(false);
		Sound_MemorySnapShot_Capture(false);
		Video_MemorySnapShot_Capture(false);
		Blitter_MemorySnapShot_Capture(false);
		DmaSnd_MemorySnapShot_Capture(false);
		Crossbar_MemorySnapShot_Capture(false);
		DSP_MemorySnapShot_Capture(false);
		DebugUI_MemorySnapShot_Capture(pszFileName, false);

		/* And close */
		MemorySnapShot_CloseFile();

		/* changes may affect also info shown in statusbar */
		Statusbar_UpdateInfo();
	}

	/* Did error? */
	if (bCaptureError)
		Log_AlertDlg(LOG_ERROR, "Unable to restore memory state from file.");
	else if (bConfirm)
		Log_AlertDlg(LOG_INFO, "Memory state file restored.");
}


/*-----------------------------------------------------------------------*/
/*
 * Save and restore functions required by the UAE CPU core...
 * ... don't use them in normal Hatari code!
 */
#include <savestate.h>

void save_u32(uae_u32 data)
{
	MemorySnapShot_Store(&data, 4);
}

void save_u16(uae_u16 data)
{
	MemorySnapShot_Store(&data, 2);
}

uae_u32 restore_u32(void)
{
	uae_u32 data;
	MemorySnapShot_Store(&data, 4);
	return data;
}

uae_u16 restore_u16(void)
{
	uae_u16 data;
	MemorySnapShot_Store(&data, 2);
	return data;
}
