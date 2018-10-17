/*  Psxle - Psx Learning Environment
 *  Copyright (C) 2018  Carlos Perez-Lopez
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <regex.h>
#include <signal.h>
#include <string>
#include <vector>
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../libpcsxcore/plugins.h"
#include "../libpcsxcore/cheat.h"
#include "../libpcsxcore/debug.h"
#include "../libpcsxcore/psxcounters.h"

// DEBUG
#define LOG_STDOUT

#define PAD_LOG  __Log
#define SIO1_LOG  __Log
#define GTE_LOG  __Log
#define CDR_LOG  __Log("%8.8lx %8.8lx: ", psxRegs.pc, psxRegs.cycle); __Log
#define CDR_LOG_IO  __Log("%8.8lx %8.8lx: ", psxRegs.pc, psxRegs.cycle); __Log

#define PSXHW_LOG   __Log("%8.8lx %8.8lx: ", psxRegs.pc, psxRegs.cycle); __Log
#define PSXBIOS_LOG __Log("%8.8lx %8.8lx: ", psxRegs.pc, psxRegs.cycle); __Log
#define PSXDMA_LOG  __Log
#define PSXMEM_LOG  __Log("%8.8lx %8.8lx: ", psxRegs.pc, psxRegs.cycle); __Log
#define PSXCPU_LOG  __Log

// Global defines
#define DEFAULT_MEM_CARD_1 "/.pcsxr/memcards/card1.mcd"
#define DEFAULT_MEM_CARD_2 "/.pcsxr/memcards/card2.mcd"
#define MEMCARD_DIR "/.pcsxr/memcards/"
//#define PLUGINS_DIR "/.pcsxr/plugins/"
#define PLUGINS_CFG_DIR "/.pcsxr/plugins/cfg/"
#define PCSXR_DOT_DIR "/.pcsxr/"
#define BIOS_DIR "/.pcsxr/bios/"
#define STATES_DIR "/.pcsxr/sstates/"
#define CHEATS_DIR "/.pcsxr/cheats/"
#define PATCHES_DIR "/.pcsxr/patches/"
// These two were added by me:
// This is likely to be right.
#define DEF_PLUGIN_DIR "/usr/lib/games/"
// This is likely to be wrong.
#define PSEMU_DATA_DIR "/usr/lib/games/psemu"

#define OLD_SLOT 1000
#define NUM_OLD_SLOTS 2
#define LAST_OLD_SLOT (OLD_SLOT + NUM_OLD_SLOTS - 1)

static char PLUGINS_DIR[] = "/.psxle/plugins";

static void CheckSubDir();

unsigned long gpuDisp;

static const int GPU_PICTURE_SIZE = 128 * 96 * 3;

int main(int argc, char* args[])
{
  if (argc < 2)
  {
    std::cout << "iso?" << std::endl;
    return -1;
  }
  char* state;
  if (argc > 2)
  {
    state = args[2];
  }
  std::string iso_filename = args[1];
  std::cout << "Setting iso file name: " << iso_filename << std::endl;
  SetIsoFile(iso_filename.c_str());
  std::cout << "Checking subdirectory" << std::endl;
	CheckSubDir();
  //std::cout << "Scanning all plugins" << std::endl;
	//ScanAllPlugins();
  std::cout << "Load plugins" << std::endl;
  std::string plugins_dir = std::string(getenv("HOME")) + PLUGINS_DIR;
  strcpy(Config.PluginsDir, plugins_dir.c_str());
  strcpy(Config.Gpu, "libDFXVideo.so");
  strcpy(Config.Spu, "libDFSound.so");
  strcpy(Config.Pad1, "libDFInput.so");
  strcpy(Config.Pad2, "libDFInput.so");
  strcpy(Config.Cdr, "libDFCdrom.so");
  strcpy(Config.Sio1, "libBladeSio1.so");
  // Using the internal simulated PSX Bios (HLE).
  // See `libpcsxcore/pxsbios.c`.
  strcpy(Config.Bios, "HLE");
  strcpy(Config.PatchesDir, PATCHES_DIR);
  // TODO: Load config from file.
  Config.Xa = 0;
  Config.SioIrq = 0;
  Config.Mdec = 0;
  Config.PsxAuto = 1;
  Config.Cdda = 0;
  Config.SlowBoot = 0;
  //Config.Dbg = 0;
  Config.PsxOut = 1;
  Config.SpuIrq = 0;
  Config.RCntFix = 0;
  Config.VSyncWA = 0;
  Config.NoMemcard = 0;
  Config.Widescreen = 0;
  Config.Cpu = 1;
  Config.PsxType = 0;
  Config.RewindCount = 0;
  Config.RewindInterval = 0;
  Config.AltSpeed1 = 50;
  Config.AltSpeed2 = 250;
  Config.HackFix = 0;
  Config.Debug = 0;
  if (LoadPlugins() == -1)
  {
    std::cout << "Could not load plugins" << std::endl;
    return -1;
  }
  // Here the window is shown.
  if (OpenPlugins() == -1)
  {
    std::cout << "Could not open plugins" << std::endl;
    return -1;
  }
  std::cout << "Initialising system" << std::endl;
	if (SysInit() == -1)
  {
    std::cout << "Could not init System" << std::endl;
    return -1;
  }
  std::cout << "Checking CD-ROM" << std::endl;
	CheckCdrom();
  std::cout << "Reseting the system" << std::endl;
	SysReset();
  std::cout << "Loading CD-ROM" << std::endl;
  if (LoadCdrom() == -1)
  {
    std::cout << "Could not load CD-ROM" << std::endl;
  }
  std::cout << "Executing ..." << std::endl;
  if (state)
  {
    LoadState(state);
  }
	//psxCpu->Execute();
  u32 previousHSyncCount = 0;
  for (;;)
  {
    psxCpu->ExecuteBlock();
    //printf("COUNTER %u\n", hSyncCount);
    if (hSyncCount < previousHSyncCount)
    {
      // This conforms one cycle.
      //         -> Action
      //   Agent
      //         <- Score
      //printf("CYCLE!\n");
      //std::cin.get();
      // The agent should act here. [pressKey/releaseKey]
      //PAD1_pressKey(0, valid_actions[key]);
      //PAD1_releaseKey(0, valid_actions[key]);
    }
    previousHSyncCount = hSyncCount;
  }
  return 0;
}

// The pcsxcore seems to make use of these functions, which are platform
// dependent. Here they are defined on top of Linux libraries.

int SysInit() {
#ifdef EMU_LOG
#ifndef LOG_STDOUT
	emuLog = fopen("emuLog.txt","wb");
#else
	emuLog = stdout;
#endif
#ifdef PSXCPU_LOG
	if (Config.PsxOut) { //PSXCPU_LOG generates so much stuff that buffer is necessary
		const int BUFSZ = 20 * 1024*1024;
		void* buf = malloc(BUFSZ);
		setvbuf(emuLog, (char*) buf, _IOFBF, BUFSZ);
	} else {
		setvbuf(emuLog, NULL, _IONBF, 0u);
	}
#else
	setvbuf(emuLog, NULL, _IONBF, 0u);
#endif
#endif

	if (EmuInit() == -1) {
		printf(_("PSX emulator couldn't be initialized.\n"));
		return -1;
	}

	LoadMcds(Config.Mcd1, Config.Mcd2);	/* TODO Do we need to have this here, or in the calling main() function?? */

	if (Config.Debug) {
    std::cout << "Starting debugger" << std::endl;
		StartDebugger();
	}

	return 0;
}

void SysReset() {
	EmuReset();
}

void SysClose() {
	EmuShutdown();
	ReleasePlugins();

	StopDebugger();

	if (emuLog != NULL) fclose(emuLog);
}

void SysPrintf(const char *fmt, ...) {
	va_list list;
	char msg[512];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	if (Config.PsxOut) {
		static char linestart = 1;
		int l = strlen(msg);

		printf(linestart ? " * %s" : "%s", msg);

		if (l > 0 && msg[l - 1] == '\n') {
			linestart = 1;
		} else {
			linestart = 0;
		}
	}

#ifdef EMU_LOG
#ifndef LOG_STDOUT
	if (emuLog != NULL) fprintf(emuLog, "%s", msg);
#endif
#endif
}

void *SysLoadLibrary(const char *lib) {
	return dlopen(lib, RTLD_NOW);
}

// HERE it is!!
//
// #define LoadSym(dest, src, name, checkerr) { \
//	dest = (src)SysLoadSym(drv, name); \
//	if (checkerr) { CheckErr(name); } else SysLibError(); \
// }
void *SysLoadSym(void *lib, const char *sym) {
	return dlsym(lib, sym);
}

const char *SysLibError() {
	return dlerror();
}

void SysMessage(const char *fmt, ...) {
	va_list list;
	char msg[512];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	if (msg[strlen(msg) - 1] == '\n')
		msg[strlen(msg) - 1] = 0;

  fprintf(stderr, "%s\n", msg);
}

void SysCloseLibrary(void *lib) {
	dlclose(lib);
}

enum {
	DKEY_SELECT = 0,
	DKEY_L3,
	DKEY_R3,
	DKEY_START,
	DKEY_UP,
	DKEY_RIGHT,
	DKEY_DOWN,
	DKEY_LEFT,
	DKEY_L2,
	DKEY_R2,
	DKEY_L1,
	DKEY_R1,
	DKEY_TRIANGLE,
	DKEY_CIRCLE,
	DKEY_CROSS,
	DKEY_SQUARE,
	DKEY_ANALOG,

	DKEY_TOTAL
};

static const std::vector<std::string> PSX_KEYS = {
  "SELECT",
  "L3",
  "R3",
  "START",
  "UP",
  "RIGHT",
  "DOWN",
  "LEFT",
  "L2",
  "R2",
  "L1",
  "R1",
  "TRIANGLE",
  "CIRCLE",
  "CROSS",
  "SQUARE",
  "ANALOG",
};

static int legal_action_set[14] = {
  1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

void SysUpdate() {
  // Press random key.
  // This should be how the agent acts on the environment.
  static int count = 1;
  static int key;
  if (count % 2 == 0)
  {
    key = legal_action_set[rand() % 14];
    std::cout << "Pressing " << PSX_KEYS[key] << std::endl;
    PAD1_pressKey(0, key);
  }
  else if (count % 2 == 1)
  {
    //std::cout << "Releasing " << PSX_KEYS[key] << std::endl;
    PAD1_releaseKey(0, key);
  }
  ++count;

  /*
  // The following block of code demnstrates how to extract screen image.
  // This would be environment that the agent sees.
  std::cout << "Getting screen pic" << std::endl;
  unsigned char gpu_picture[386 * 480 * 3]; // 128 * 96 * 3
  //GPU_getScreenPic(gpu_picture);
  GPU_getPSXScreen(gpu_picture);
  std::ofstream fout;
  fout.open("screen_pic");
  fout.write((char*) gpu_picture, sizeof(gpu_picture));
  fout.close();
  */

  /*
  // Handling user input.
	PADhandleKey(PAD1_keypressed() );
	PADhandleKey(PAD2_keypressed() );

	SysDisableScreenSaver();
  */
}

void SysRunGui() {
  std::cout << "SysRunGui? No GUI!" << std::endl;
}

// These functions are used by the main.

/* Create a directory under the $HOME directory, if that directory doesn't already exist */
static void CreateHomeConfigDir(char *directory) {
	struct stat buf;

	if (stat(directory, &buf) == -1) {
		gchar *dir_name = g_build_filename (getenv("HOME"), directory, NULL);
		mkdir(dir_name, S_IRWXU | S_IRWXG);
		g_free (dir_name);
	}
}

static void CheckSubDir() {
	// make sure that ~/.pcsxr exists
	CreateHomeConfigDir(PCSXR_DOT_DIR);

	CreateHomeConfigDir(BIOS_DIR);
	CreateHomeConfigDir(MEMCARD_DIR);
	CreateHomeConfigDir(STATES_DIR);
	CreateHomeConfigDir(PLUGINS_DIR);
	CreateHomeConfigDir(PLUGINS_CFG_DIR);
	CreateHomeConfigDir(CHEATS_DIR);
	CreateHomeConfigDir(PATCHES_DIR);
}

int match(const char *string, char *pattern) {
	int    status;
	regex_t    re;

	if (regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB) != 0) {
		return 0;
	}
	status = regexec(&re, string, (size_t) 0, NULL, 0);
	regfree(&re);
	if (status != 0) {
		return 0;
	}

	return 1;
}

/*
static void ScanBios(gchar* scandir) {
	// scan for bioses
	DIR *dir;
	struct dirent *ent;

	gchar *linkname;
	gchar *filename;

	 Any bioses found will be symlinked to the following directory 
	dir = opendir(scandir);
	if (dir != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			filename = g_build_filename(scandir, ent->d_name, NULL);

			if (match(filename, ".*\\.bin$") == 0 &&
				match(filename, ".*\\.BIN$") == 0) {
				continue;	* Skip this file *
			} else {
				/* Create a symlink from this file to the directory ~/.pcsxr/plugin *
				linkname = g_build_filename(getenv("HOME"), BIOS_DIR, ent->d_name, NULL);
        std::cout << filename << " " << linkname << std::endl;
				symlink(filename, linkname);

				g_free(linkname);
			}
			g_free(filename);
		}
		closedir(dir);
	}
}
*/
static void CheckSymlinksInPath(char* dotdir) {
	DIR *dir;
	struct dirent *ent;
	struct stat stbuf;
	gchar *linkname;

	dir = opendir(dotdir);
	if (dir == NULL) {
		SysMessage(_("Could not open directory: '%s'\n"), dotdir);
		return;
	}

	/* Check for any bad links in the directory. If the remote
	   file no longer exists, remove the link */
	while ((ent = readdir(dir)) != NULL) {
		linkname = g_strconcat (dotdir, ent->d_name, NULL);

		if (stat(linkname, &stbuf) == -1) {
			/* File link is bad, remove it */
			unlink(linkname);
		}
		g_free (linkname);
	}
	closedir(dir);
}

void OnFile_Exit()
{
  std::cout << "Bye!" << std::endl;
  exit(0);
}

void SignalExit(int sig) {
	ClosePlugins();
	OnFile_Exit();
}

#define PARSEPATH(dst, src) \
	ptr = src + strlen(src); \
	while (*ptr != '\\' && ptr != src) ptr--; \
	if (ptr != src) { \
		strcpy(dst, ptr+1); \
	}


int _OpenPlugins() {
	int ret;

	signal(SIGINT, SignalExit);
	signal(SIGPIPE, SignalExit);

	GPU_clearDynarec(clearDynarec);

	ret = CDR_open();
	if (ret < 0) { SysMessage(_("Error opening CD-ROM plugin!")); return -1; }
	ret = SPU_open();
	if (ret < 0) { SysMessage(_("Error opening SPU plugin!")); return -1; }
	SPU_registerCallback(SPUirq);
	ret = GPU_open(&gpuDisp, "PSXLE", NULL);
	if (ret < 0) { SysMessage(_("Error opening GPU plugin!")); return -1; }
	ret = PAD1_open(&gpuDisp);
	ret |= PAD1_init(1); // Allow setting to change during run
	if (ret < 0) { SysMessage(_("Error opening Controller 1 plugin!")); return -1; }
	PAD1_registerVibration(GPU_visualVibration);
	PAD1_registerCursor(GPU_cursor);
	ret = PAD2_open(&gpuDisp);
	ret |= PAD2_init(2); // Allow setting to change during run
	if (ret < 0) { SysMessage(_("Error opening Controller 2 plugin!")); return -1; }
	PAD2_registerVibration(GPU_visualVibration);
	PAD2_registerCursor(GPU_cursor);
#ifdef ENABLE_SIO1API
	ret = SIO1_open(&gpuDisp);
	if (ret < 0) { SysMessage(_("Error opening SIO1 plugin!")); return -1; }
	SIO1_registerCallback(SIO1irq);
#endif

	if (Config.UseNet && !NetOpened) {
		netInfo info;
		char path[MAXPATHLEN];
		char dotdir[MAXPATHLEN];

		strncpy(dotdir, getenv("HOME"), MAXPATHLEN-100);
		strcat(dotdir, "/.pcsxr/plugins/");

		strcpy(info.EmuName, "PCSXR " PACKAGE_VERSION);
		strncpy(info.CdromID, CdromId, 9);
		strncpy(info.CdromLabel, CdromLabel, 9);
		info.psxMem = psxM;
		info.GPU_showScreenPic = GPU_showScreenPic;
		info.GPU_displayText = GPU_displayText;
		info.GPU_showScreenPic = GPU_showScreenPic;
		info.PAD_setSensitive = PAD1_setSensitive;
		sprintf(path, "%s%s", Config.BiosDir, Config.Bios);
		strcpy(info.BIOSpath, path);
		strcpy(info.MCD1path, Config.Mcd1);
		strcpy(info.MCD2path, Config.Mcd2);
		sprintf(path, "%s%s", dotdir, Config.Gpu);
		strcpy(info.GPUpath, path);
		sprintf(path, "%s%s", dotdir, Config.Spu);
		strcpy(info.SPUpath, path);
		sprintf(path, "%s%s", dotdir, Config.Cdr);
		strcpy(info.CDRpath, path);
		NET_setInfo(&info);

		ret = NET_open(&gpuDisp);
		if (ret < 0) {
			if (ret == -2) {
				// -2 is returned when something in the info
				// changed and needs to be synced
				char *ptr;

				PARSEPATH(Config.Bios, info.BIOSpath);
				PARSEPATH(Config.Gpu,  info.GPUpath);
				PARSEPATH(Config.Spu,  info.SPUpath);
				PARSEPATH(Config.Cdr,  info.CDRpath);

				strcpy(Config.Mcd1, info.MCD1path);
				strcpy(Config.Mcd2, info.MCD2path);
				return -2;
			} else {
				Config.UseNet = FALSE;
			}
		} else {
			if (NET_queryPlayer() == 1) {
				if (SendPcsxInfo() == -1) Config.UseNet = FALSE;
			} else {
				if (RecvPcsxInfo() == -1) Config.UseNet = FALSE;
			}
		}
		NetOpened = TRUE;
	} else if (Config.UseNet) {
		NET_resume();
	}

	return 0;
}

int OpenPlugins() {
	int ret;

	while ((ret = _OpenPlugins()) == -2) {
		ReleasePlugins();
		LoadMcds(Config.Mcd1, Config.Mcd2);
		if (LoadPlugins() == -1) return -1;
	}
	return ret;
}

void ClosePlugins() {
	int ret;

	signal(SIGINT, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
	ret = CDR_close();
	if (ret < 0) { SysMessage(_("Error closing CD-ROM plugin!")); return; }
	ret = SPU_close();
	if (ret < 0) { SysMessage(_("Error closing SPU plugin!")); return; }
	ret = PAD1_close();
	if (ret < 0) { SysMessage(_("Error closing Controller 1 Plugin!")); return; }
	ret = PAD2_close();
	if (ret < 0) { SysMessage(_("Error closing Controller 2 plugin!")); return; }
	ret = GPU_close();
	if (ret < 0) { SysMessage(_("Error closing GPU plugin!")); return; }
#ifdef ENABLE_SIO1API
	ret = SIO1_close();
	if (ret < 0) { SysMessage(_("Error closing SIO1 plugin!")); return; }
#endif

	if (Config.UseNet) {
		NET_pause();
	}
}

gchar* get_cdrom_label_trim() {
	char trimlabel[33];
	int j;

	strncpy(trimlabel, CdromLabel, 32);
	trimlabel[32] = 0;
	for (j = 31; j >= 0; j--) {
		if (trimlabel[j] == ' ')
			trimlabel[j] = 0;
		else
			continue;
	}

	return g_strdup(trimlabel);
}

gchar* get_cdrom_label_id(const gchar* suffix) {
	const u8 lblmax = sizeof(CdromId) + sizeof(CdromLabel) + 20u;
	//printf("MAx %u\n", lblmax);
	char buf[lblmax];
	gchar *trimlabel = get_cdrom_label_trim();

	snprintf(buf, lblmax, "%.32s-%.9s%s", trimlabel, CdromId, suffix);

	g_free(trimlabel);

	if (strlen(buf) <= (2+strlen(dot_extension_cht)))
		return g_strconcat("psx-default", dot_extension_cht, NULL);
	else 
		return g_strdup(buf);
}

void autoloadCheats() {
	ClearAllCheats();
	gchar *chtfile = get_cdrom_label_id(dot_extension_cht);
	gchar *defaultChtFilePath = g_build_filename (getenv("HOME"), CHEATS_DIR, chtfile, NULL);
	LoadCheats(defaultChtFilePath); // file existence/access check in LoadCheats()
	g_free(defaultChtFilePath);
	g_free(chtfile);
}
