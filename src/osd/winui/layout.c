// For licensing and usage information, read docs/winui_license.txt
//****************************************************************************

/***************************************************************************

  layout.c

  MAME specific TreeView definitions (and maybe more in the future)

***************************************************************************/
// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <commctrl.h>

// standard C headers
#include <stdio.h>  /* for sprintf */
#include <stdlib.h> /* For malloc and free */
#include <string.h>

// MAME/MAMEUI headers
#include "bitmask.h"
#include "treeview.h"
#include "emu.h"
#include "mui_util.h"
#include "resource.h"
#include "directories.h"
#include "mui_opts.h"
#include "splitters.h"
#include "help.h"
#include "mui_audit.h"
#include "winui.h"
#include "properties.h"

static BOOL FilterAvailable(int driver_index);

extern const FOLDERDATA g_folderData[] =
{
	{"All Games",       "allgames",          FOLDER_ALLGAMES,     IDI_FOLDER,               0,             0,            NULL,                       NULL,                    TRUE },
	{"Available",       "available",         FOLDER_AVAILABLE,    IDI_FOLDER_AVAILABLE,     F_AVAILABLE,   0,            NULL,                       FilterAvailable,         TRUE },
#ifdef SHOW_UNAVAILABLE_FOLDER
	{"Unavailable",     "unavailable",       FOLDER_UNAVAILABLE,  IDI_FOLDER_UNAVAILABLE,   0,             F_AVAILABLE,  NULL,                       FilterAvailable,         FALSE },
#endif
	{"Manufacturer",    "manufacturer",      FOLDER_MANUFACTURER, IDI_FOLDER_MANUFACTURER,  0,             0,            CreateManufacturerFolders },
	{"Year",            "year",              FOLDER_YEAR,         IDI_FOLDER_YEAR,          0,             0,            CreateYearFolders },
	{"Source",          "source",            FOLDER_SOURCE,       IDI_FOLDER_SOURCE,        0,             0,            CreateSourceFolders },
//	{"CPU",             "cpu",               FOLDER_CPU,          IDI_CPU,                  0,             0,            CreateCPUFolders },
//	{"Sound",           "sound",             FOLDER_SND,          IDI_SOUND,                0,             0,            CreateSoundFolders },
	{"BIOS",            "bios",              FOLDER_BIOS,         IDI_FOLDER,               0,             0,            CreateBIOSFolders,          DriverIsBios,            TRUE },
	{"CHD",             "harddisk",          FOLDER_HARDDISK,     IDI_HARDDISK,             0,             0,            NULL,                       DriverIsHarddisk,        TRUE },
	{"Samples",         "samples",           FOLDER_SAMPLES,      IDI_FOLDER,               0,             0,            NULL,                       DriverUsesSamples,       TRUE },
//	{"Dumping Status",  "dumping",           FOLDER_DUMPING,      IDI_FOLDER,               0,             0,            CreateDumpingFolders },
	{"Working",         "working",           FOLDER_WORKING,      IDI_WORKING,              F_WORKING,     F_NONWORKING, NULL,                       DriverIsBroken,          FALSE },
	{"Imperfect",       "imperfect",         FOLDER_DEFICIENCY,   IDI_FOLDER,               0,             0,            CreateDeficiencyFolders },
	{"Not Working",     "nonworking",        FOLDER_NONWORKING,   IDI_NONWORKING,           F_NONWORKING,  F_WORKING,    NULL,                       DriverIsBroken,          TRUE },
	{"Originals",       "originals",         FOLDER_ORIGINAL,     IDI_FOLDER,               F_ORIGINALS,   F_CLONES,     NULL,                       DriverIsClone,           FALSE },
	{"Clones",          "clones",            FOLDER_CLONES,       IDI_FOLDER,               F_CLONES,      F_ORIGINALS,  NULL,                       DriverIsClone,           TRUE },
	{"Screens",         "screens",           FOLDER_SCREENS,      IDI_FOLDER,               0,             0,            CreateScreenFolders },
//	{"Resolution",      "resolution",        FOLDER_RESOLUTION,   IDI_FOLDER,               0,             0,            CreateResolutionFolders },
//	{"FPS",             "fps",               FOLDER_FPS,          IDI_FOLDER,               0,             0,            CreateFPSFolders },
	{"Horizontal",      "horizontal",        FOLDER_HORIZONTAL,   IDI_FOLDER_HORIZONTAL,    F_HORIZONTAL,  F_VERTICAL,   NULL,                       DriverIsVertical,        FALSE },
	{"Vertical",        "vertical",          FOLDER_VERTICAL,     IDI_FOLDER_VERTICAL,      F_VERTICAL,    F_HORIZONTAL, NULL,                       DriverIsVertical,        TRUE },
	{"Raster",          "raster",            FOLDER_RASTER,       IDI_FOLDER,               F_RASTER,      F_VECTOR,     NULL,                       DriverIsVector,          FALSE },
	{"Vector",          "vector",            FOLDER_VECTOR,       IDI_FOLDER,               F_VECTOR,      F_RASTER,     NULL,                       DriverIsVector,          TRUE },
	{"Trackball",       "trackball",         FOLDER_TRACKBALL,    IDI_FOLDER,               0,             0,            NULL,                       DriverUsesTrackball,     TRUE },
	{"Lightgun",        "Lightgun",          FOLDER_LIGHTGUN,     IDI_FOLDER,               0,             0,            NULL,                       DriverUsesLightGun,      TRUE },
	{"Stereo",          "stereo",            FOLDER_STEREO,       IDI_FOLDER,               0,             0,            NULL,                       DriverIsStereo,          TRUE },
	{"Save State",      "savestate",         FOLDER_SAVESTATE,    IDI_FOLDER,               0,             0,            NULL,                       DriverSupportsSaveState, TRUE },
	{"Mechanical",      "mechanical",        FOLDER_MECHANICAL,   IDI_FOLDER,               0,             0,            NULL,                       DriverIsMechanical,      TRUE },
	{"Non Mechanical",  "nonmechanical",     FOLDER_NONMECHANICAL,IDI_FOLDER,               0,             0,            NULL,                       DriverIsMechanical,      FALSE },
	{ NULL }
};

/* list of filter/control Id pairs */
extern const FILTER_ITEM g_filterList[] =
{
	{ F_CLONES,       IDC_FILTER_CLONES,      DriverIsClone, TRUE },
	{ F_NONWORKING,   IDC_FILTER_NONWORKING,  DriverIsBroken, TRUE },
	{ F_UNAVAILABLE,  IDC_FILTER_UNAVAILABLE, FilterAvailable, FALSE },
	{ F_RASTER,       IDC_FILTER_RASTER,      DriverIsVector, FALSE },
	{ F_VECTOR,       IDC_FILTER_VECTOR,      DriverIsVector, TRUE },
	{ F_ORIGINALS,    IDC_FILTER_ORIGINALS,   DriverIsClone, FALSE },
	{ F_WORKING,      IDC_FILTER_WORKING,     DriverIsBroken, FALSE },
	{ F_AVAILABLE,    IDC_FILTER_AVAILABLE,   FilterAvailable, TRUE },
	{ F_HORIZONTAL,   IDC_FILTER_HORIZONTAL,  DriverIsVertical, FALSE },
	{ F_VERTICAL,     IDC_FILTER_VERTICAL,    DriverIsVertical, TRUE },
	{ F_MECHANICAL,   IDC_FILTER_MECHANICAL,  DriverIsMechanical, TRUE },
	{ F_ARCADE,       IDC_FILTER_ARCADE,      DriverIsArcade, TRUE },
	{ F_MESS,         IDC_FILTER_MESS,        DriverIsArcade, FALSE },
	{ 0 }
};

extern const SPLITTERINFO g_splitterInfo[] =
{
	{ 0.25,	IDC_SPLITTER,	IDC_TREE,	IDC_LIST,		AdjustSplitter1Rect },
	{ 0.5,	IDC_SPLITTER2,	IDC_LIST,	IDC_SSFRAME,	AdjustSplitter2Rect },
	{ -1 }
};

extern const MAMEHELPINFO g_helpInfo[] =
{
	{ ID_HELP_CONTENTS,		TRUE,	MAMEUIHELP "::/html/mameui_overview.htm" },
	{ ID_HELP_WHATS_NEWUI,	TRUE,	MAMEUIHELP "::/html/mameui_changes.txt" },
	{ ID_HELP_TROUBLE,		TRUE,	MAMEUIHELP "::/html/mameui_support.htm" },
	{ ID_HELP_RELEASE,		FALSE,	"windows.txt" },
	{ ID_HELP_WHATS_NEW,	TRUE,	MAMEUIHELP "::/docs/whatsnew.txt" },
	{ -1 }
};

extern const PROPERTYSHEETINFO g_propSheets[] =
{
	{ FALSE,	NULL,				IDD_PROP_GAME,			GamePropertiesDialogProc },
	{ FALSE,	NULL,				IDD_PROP_AUDIT,			GameAuditDialogProc },
	{ TRUE,		NULL,				IDD_PROP_DISPLAY,		GameOptionsProc },
	{ TRUE,		NULL,				IDD_PROP_ADVANCED,		GameOptionsProc },
	{ TRUE,		NULL,				IDD_PROP_SCREEN,		GameOptionsProc },
	{ TRUE,		NULL,				IDD_PROP_SOUND,			GameOptionsProc },
	{ TRUE,		NULL,				IDD_PROP_INPUT,			GameOptionsProc },
	{ TRUE,		NULL,				IDD_PROP_CONTROLLER,	GameOptionsProc },
	{ TRUE,		NULL,				IDD_PROP_MISC,			GameOptionsProc },
	{ TRUE,		NULL,				IDD_PROP_DEBUG,			GameOptionsProc },
	{ TRUE,		DriverIsVector,		IDD_PROP_VECTOR,		GameOptionsProc },
	{ FALSE }
};

extern const ICONDATA g_iconData[] =
{
	{ IDI_WIN_NOROMS,			"noroms" },
	{ IDI_WIN_ROMS,				"roms" },
	{ IDI_WIN_UNKNOWN,			"unknown" },
	{ IDI_WIN_CLONE,			"clone" },
	{ IDI_WIN_REDX,				"warning" },
	{ 0 }
};
extern const TCHAR g_szPlayGameString[] = TEXT("&Play %s");
extern const char g_szGameCountString[] = "%d games";
extern const char g_szHistoryFileName[] = "history.dat";
extern const char g_szMameInfoFileName[] = "mameinfo.dat";

static BOOL FilterAvailable(int driver_index)
{
	return IsAuditResultYes(GetRomAuditResults(driver_index));
}
