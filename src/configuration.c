/*
  Hatari

  Configuration File

  The configuration file istored in a binary format to prevent tampering.
        We also store the version number in the file to prevent people from
        copying old .cfg files between versions.
*/

#include "main.h"
#include "configuration.h"
#include "dialog.h"
#include "video.h"
#include "view.h"
#include "vdi.h"
#include "screen.h"

static FILE *ConfigFile;
/*static OFSTRUCT ConfigFileInfo;*/
BOOL bFirstTimeInstall=FALSE;              /* Has been run before? Used to set default joysticks etc... */


/*-----------------------------------------------------------------------*/
/*
  Set default configuration values
  This is new in Hatari - Winston always loaded its values from a config
  file. But since Hatari does not yet use a config file, we need this!
*/
void Configuration_SetDefault(void)
{
  ConfigureParams.Sound.bEnableSound = FALSE;
  ConfigureParams.Screen.Advanced.bFrameSkip = FALSE;
  ConfigureParams.Screen.Advanced.bAllowOverscan = TRUE;
  ConfigureParams.Screen.ChosenDisplayMode = DISPLAYMODE_HICOL_LOWRES;
}


/*-----------------------------------------------------------------------*/
/*
  Load program setting from configuration file
*/
void Configuration_Init(void)
{
/*
  char sVersionString[VERSION_STRING_SIZE];
  int i,j;

  // Set default settings, incase registry does not exist or is invalid
  Dialog_DefaultConfigurationDetails();
  View_DefaultWindowPos();

  // Open configuration file
  if (Configuration_OpenFileToRead()) {
    // Version, check matches
    Configuration_ReadFromFile(sVersionString,VERSION_STRING_SIZE);
    if (memcmp(sVersionString,VERSION_STRING,VERSION_STRING_SIZE)==0) {
      // Configure
      Configuration_ReadFromFile(&ConfigureParams.Configure.nMinMaxSpeed,4);
      Configuration_ReadFromFile(&WindowInitRect.left,4);
      Configuration_ReadFromFile(&WindowInitRect.top,4);
      for(i=0; i<2; i++) {
        for(j=0; j<MAX_FLOPPY_MENU_IMAGES; j++) {
          Configuration_ReadFromFile(szPreviousImageFilenames[i][j],MAX_FILENAME_LENGTH);
        }
        Configuration_ReadFromFile(&nPreviousImageFilenames[i],4);
      }
      for(i=0; i<MAX_TOSIMAGE_COMBO_IMAGES; i++)
        Configuration_ReadFromFile(szComboTOSImages[i],MAX_FILENAME_LENGTH);
      // Screen
      Configuration_ReadFromFile(&ConfigureParams.Screen.bFullScreen,4);
      Configuration_ReadFromFile(&ConfigureParams.Screen.Advanced.bDoubleSizeWindow,4);
      Configuration_ReadFromFile(&ConfigureParams.Screen.Advanced.bAllowOverscan,4);
      Configuration_ReadFromFile(&ConfigureParams.Screen.Advanced.bInterlacedFullScreen,4);
      Configuration_ReadFromFile(&ConfigureParams.Screen.Advanced.bSyncToRetrace,4);
      Configuration_ReadFromFile(&ConfigureParams.Screen.ChosenDisplayMode,4);
      Configuration_ReadFromFile(&ConfigureParams.Screen.bCaptureChange,4);
      Configuration_ReadFromFile(&ConfigureParams.Screen.nFramesPerSecond,4);
      Configuration_ReadFromFile(&ConfigureParams.Screen.bUseHighRes,4);
      // Joysticks    
      Configuration_ReadFromFile(&ConfigureParams.Joysticks.bUseDirectInput,4);
      Configuration_ReadFromFile(&ConfigureParams.Joysticks.Joy[0].bCursorEmulation,4);
      Configuration_ReadFromFile(&ConfigureParams.Joysticks.Joy[0].bEnableAutoFire,4);
      Configuration_ReadFromFile(&ConfigureParams.Joysticks.Joy[1].bCursorEmulation,4);
      Configuration_ReadFromFile(&ConfigureParams.Joysticks.Joy[1].bEnableAutoFire,4);
      // Keyboard
      Configuration_ReadFromFile(&ConfigureParams.Keyboard.bDisableKeyRepeat,4);
      Configuration_ReadFromFile(&ConfigureParams.Keyboard.ShortCuts[SHORT_CUT_F11][SHORT_CUT_SHIFT],4);
      Configuration_ReadFromFile(&ConfigureParams.Keyboard.ShortCuts[SHORT_CUT_F11][SHORT_CUT_CTRL],4);
      Configuration_ReadFromFile(&ConfigureParams.Keyboard.ShortCuts[SHORT_CUT_F12][SHORT_CUT_SHIFT],4);
      Configuration_ReadFromFile(&ConfigureParams.Keyboard.ShortCuts[SHORT_CUT_F12][SHORT_CUT_CTRL],4);
      Configuration_ReadFromFile(ConfigureParams.Keyboard.szMappingFileName,sizeof(ConfigureParams.Keyboard.szMappingFileName));
      // Sound
      Configuration_ReadFromFile(&ConfigureParams.Sound.bEnableSound,4);
      Configuration_ReadFromFile(&ConfigureParams.Sound.nPlaybackQuality,4);
      Configuration_ReadFromFile(ConfigureParams.Sound.szYMCaptureFileName,sizeof(ConfigureParams.Sound.szYMCaptureFileName));
      // Memory
      Configuration_ReadFromFile(&ConfigureParams.Memory.nMemorySize,4);
      Configuration_ReadFromFile(ConfigureParams.Memory.szMemoryCaptureFileName,sizeof(ConfigureParams.Memory.szMemoryCaptureFileName));
      // DiscImage
      Configuration_ReadFromFile(&ConfigureParams.DiscImage.bAutoInsertDiscB,4);
      Configuration_ReadFromFile(ConfigureParams.DiscImage.szDiscImageDirectory,sizeof(ConfigureParams.DiscImage.szDiscImageDirectory));
      // HardDisc
      Configuration_ReadFromFile(&ConfigureParams.HardDisc.nDriveList,4);
      Configuration_ReadFromFile(&ConfigureParams.HardDisc.bBootFromHardDisc,4);
      Configuration_ReadFromFile(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_C],sizeof(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_C]));
      Configuration_ReadFromFile(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_D],sizeof(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_D]));
      Configuration_ReadFromFile(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_E],sizeof(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_E]));
      Configuration_ReadFromFile(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_F],sizeof(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_F]));
      // TOSGEM
      Configuration_ReadFromFile(ConfigureParams.TOSGEM.szTOSImageFileName,sizeof(ConfigureParams.TOSGEM.szTOSImageFileName));
      Configuration_ReadFromFile(&ConfigureParams.TOSGEM.bUseTimeDate,4);
      Configuration_ReadFromFile(&ConfigureParams.TOSGEM.bAccGEMGraphics,4);
      Configuration_ReadFromFile(&ConfigureParams.TOSGEM.bUseExtGEMResolutions,4);
      Configuration_ReadFromFile(&ConfigureParams.TOSGEM.nGEMResolution,4);
      Configuration_ReadFromFile(&ConfigureParams.TOSGEM.nGEMColours,4);
      // RS232
      Configuration_ReadFromFile(&ConfigureParams.RS232.bEnableRS232,4);
      Configuration_ReadFromFile(&ConfigureParams.RS232.nCOMPort,4);
      // Printer
      Configuration_ReadFromFile(&ConfigureParams.Printer.bEnablePrinting,4);
      Configuration_ReadFromFile(&ConfigureParams.Printer.bPrintToFile,4);
      Configuration_ReadFromFile(ConfigureParams.Printer.szPrintToFileName,sizeof(ConfigureParams.Printer.szPrintToFileName));
      // Favourites
      Configuration_ReadFromFile(&ConfigureParams.Favourites.bCheckDiscs,4);
      Configuration_ReadFromFile(&ConfigureParams.Favourites.bOnlyShowIfExist,4);

      bUseVDIRes = ConfigureParams.TOSGEM.bUseExtGEMResolutions;
      bUseHighRes = ConfigureParams.Screen.bUseHighRes || (bUseVDIRes && (ConfigureParams.TOSGEM.nGEMColours==GEMCOLOUR_2));
    }

    // And close up
    Configuration_CloseFile();
  }
  else {
    // No configuration file, assume first-time install
    bFirstTimeInstall = TRUE;
  }

  // Copy details to globals, TRUE
  Dialog_CopyDetailsFromConfiguration(TRUE);
*/
}


/*-----------------------------------------------------------------------*/
/*
  Save program setting to configuration file
*/
void Configuration_UnInit(void)
{
/* FIXME: Rewrite this, too! */
/*
  int i,j;

  // Open configuration file
  if (Configuration_OpenFileToWrite()) {
    // Version
    Configuration_WriteToFile(VERSION_STRING,VERSION_STRING_SIZE);
    // Configure
    Configuration_WriteToFile(&ConfigureParams.Configure.nMinMaxSpeed,4);
    ConfigureParams.Configure.nPrevMinMaxSpeed = ConfigureParams.Configure.nMinMaxSpeed;
    Configuration_WriteToFile(&WindowInitRect.left,4);
    Configuration_WriteToFile(&WindowInitRect.top,4);
    for(i=0; i<2; i++) {
      for(j=0; j<MAX_FLOPPY_MENU_IMAGES; j++) {
        Configuration_WriteToFile(szPreviousImageFilenames[i][j],MAX_FILENAME_LENGTH);
      }
      Configuration_WriteToFile(&nPreviousImageFilenames[i],4);
    }
    for(i=0; i<MAX_TOSIMAGE_COMBO_IMAGES; i++)
      Configuration_WriteToFile(szComboTOSImages[i],MAX_FILENAME_LENGTH);
    // Screen
    Configuration_WriteToFile(&ConfigureParams.Screen.bFullScreen,4);
    Configuration_WriteToFile(&ConfigureParams.Screen.Advanced.bDoubleSizeWindow,4);
    Configuration_WriteToFile(&ConfigureParams.Screen.Advanced.bAllowOverscan,4);
    Configuration_WriteToFile(&ConfigureParams.Screen.Advanced.bInterlacedFullScreen,4);
    Configuration_WriteToFile(&ConfigureParams.Screen.Advanced.bSyncToRetrace,4);
    Configuration_WriteToFile(&ConfigureParams.Screen.ChosenDisplayMode,4);
    Configuration_WriteToFile(&ConfigureParams.Screen.bCaptureChange,4);
    Configuration_WriteToFile(&ConfigureParams.Screen.nFramesPerSecond,4);
    Configuration_WriteToFile(&ConfigureParams.Screen.bUseHighRes,4);
    // Joysticks    
    Configuration_WriteToFile(&ConfigureParams.Joysticks.bUseDirectInput,4);
    Configuration_WriteToFile(&ConfigureParams.Joysticks.Joy[0].bCursorEmulation,4);
    Configuration_WriteToFile(&ConfigureParams.Joysticks.Joy[0].bEnableAutoFire,4);
    Configuration_WriteToFile(&ConfigureParams.Joysticks.Joy[1].bCursorEmulation,4);
    Configuration_WriteToFile(&ConfigureParams.Joysticks.Joy[1].bEnableAutoFire,4);
    // Keyboard
    Configuration_WriteToFile(&ConfigureParams.Keyboard.bDisableKeyRepeat,4);
    Configuration_WriteToFile(&ConfigureParams.Keyboard.ShortCuts[SHORT_CUT_F11][SHORT_CUT_SHIFT],4);
    Configuration_WriteToFile(&ConfigureParams.Keyboard.ShortCuts[SHORT_CUT_F11][SHORT_CUT_CTRL],4);
    Configuration_WriteToFile(&ConfigureParams.Keyboard.ShortCuts[SHORT_CUT_F12][SHORT_CUT_SHIFT],4);
    Configuration_WriteToFile(&ConfigureParams.Keyboard.ShortCuts[SHORT_CUT_F12][SHORT_CUT_CTRL],4);
    Configuration_WriteToFile(ConfigureParams.Keyboard.szMappingFileName,sizeof(ConfigureParams.Keyboard.szMappingFileName));
    // Sound
    Configuration_WriteToFile(&ConfigureParams.Sound.bEnableSound,4);
    Configuration_WriteToFile(&ConfigureParams.Sound.nPlaybackQuality,4);
    Configuration_WriteToFile(ConfigureParams.Sound.szYMCaptureFileName,sizeof(ConfigureParams.Sound.szYMCaptureFileName));
    // Memory
    Configuration_WriteToFile(&ConfigureParams.Memory.nMemorySize,4);
    Configuration_WriteToFile(ConfigureParams.Memory.szMemoryCaptureFileName,sizeof(ConfigureParams.Memory.szMemoryCaptureFileName));
    // DiscImage
    Configuration_WriteToFile(&ConfigureParams.DiscImage.bAutoInsertDiscB,4);
    Configuration_WriteToFile(ConfigureParams.DiscImage.szDiscImageDirectory,sizeof(ConfigureParams.DiscImage.szDiscImageDirectory));
    // HardDisc
    Configuration_WriteToFile(&ConfigureParams.HardDisc.nDriveList,4);
    Configuration_WriteToFile(&ConfigureParams.HardDisc.bBootFromHardDisc,4);
    Configuration_WriteToFile(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_C],sizeof(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_C]));
    Configuration_WriteToFile(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_D],sizeof(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_D]));
    Configuration_WriteToFile(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_E],sizeof(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_E]));
    Configuration_WriteToFile(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_F],sizeof(ConfigureParams.HardDisc.szHardDiscDirectories[DRIVE_F]));
    // TOSGEM
    Configuration_WriteToFile(ConfigureParams.TOSGEM.szTOSImageFileName,sizeof(ConfigureParams.TOSGEM.szTOSImageFileName));
    Configuration_WriteToFile(&ConfigureParams.TOSGEM.bUseTimeDate,4);
    Configuration_WriteToFile(&ConfigureParams.TOSGEM.bAccGEMGraphics,4);
    Configuration_WriteToFile(&ConfigureParams.TOSGEM.bUseExtGEMResolutions,4);
    Configuration_WriteToFile(&ConfigureParams.TOSGEM.nGEMResolution,4);
    Configuration_WriteToFile(&ConfigureParams.TOSGEM.nGEMColours,4);
    // RS232
    Configuration_WriteToFile(&ConfigureParams.RS232.bEnableRS232,4);
    Configuration_WriteToFile(&ConfigureParams.RS232.nCOMPort,4);
    // Printer
    Configuration_WriteToFile(&ConfigureParams.Printer.bEnablePrinting,4);
    Configuration_WriteToFile(&ConfigureParams.Printer.bPrintToFile,4);
    Configuration_WriteToFile(ConfigureParams.Printer.szPrintToFileName,sizeof(ConfigureParams.Printer.szPrintToFileName));
    // Favourites
    Configuration_WriteToFile(&ConfigureParams.Favourites.bCheckDiscs,4);
    Configuration_WriteToFile(&ConfigureParams.Favourites.bOnlyShowIfExist,4);

    // And close up
    Configuration_CloseFile();
  }
*/
}


/*-----------------------------------------------------------------------*/
/*
  Open configuration file to write to
*/
BOOL Configuration_OpenFileToWrite(void)
{
  char szString[MAX_FILENAME_LENGTH];

  /* Create file */
  sprintf(szString,"%s/hatari.cfg",szWorkingDir);
  ConfigFile = fopen(szString, "wb");
  if (ConfigFile!=NULL)
    return(TRUE);

  /* Whoops, error */
  return(FALSE);
}


/*-----------------------------------------------------------------------*/
/*
  Open configuration file for reading
*/
BOOL Configuration_OpenFileToRead(void)
{
  char szString[MAX_FILENAME_LENGTH];

  /* Create file */
  sprintf(szString,"%s/hatari.cfg",szWorkingDir);
  ConfigFile = fopen(szString, "rb");
  if (ConfigFile!=NULL)
    return(TRUE);

  /* Whoops, error */
  return(FALSE);
}


/*-----------------------------------------------------------------------*/
/*
  Close configuration
*/
void Configuration_CloseFile(void)
{
  fclose(ConfigFile);
}


/*-----------------------------------------------------------------------*/
/*
  Write entry to configuration file
*/
void Configuration_WriteToFile(void *pData,int nBytes)
{
  fwrite(pData, 1, nBytes, ConfigFile);
}


/*-----------------------------------------------------------------------*/
/*
  Read entry from configuration file
*/
void Configuration_ReadFromFile(void *pData,int nBytes)
{
  fread(pData, 1, nBytes, ConfigFile);
}
