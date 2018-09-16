#include <dlfcn.h>
#include <iostream>
#include <regex.h>
#include <signal.h>
#include <string>
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../libpcsxcore/plugins.h"

#define DEFAULT_MEM_CARD_1 "/.pcsxr/memcards/card1.mcd"
#define DEFAULT_MEM_CARD_2 "/.pcsxr/memcards/card2.mcd"
#define MEMCARD_DIR "/.pcsxr/memcards/"
#define PLUGINS_DIR "/.pcsxr/plugins/"
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


static void CheckSubDir();
static void ScanAllPlugins(void);

int main(int argc, char* args[])
{
  if (argc < 2)
  {
    std::cout << "iso?" << std::endl;
    return -1;
  }
  std::string iso_filename = args[1];
  std::cout << "Setting iso file name: " << iso_filename << std::endl;
  SetIsoFile(iso_filename.c_str());
  std::cout << "Checking subdirectory" << std::endl;
	CheckSubDir();
  std::cout << "Scanning all plugins" << std::endl;
	ScanAllPlugins();
  std::cout << "Initialising system" << std::endl;
	if (SysInit() == -1)
  {
    std::cout << "Could not init System" << std::endl;
    return 1;
  }
  std::cout << "Checking CD-ROM" << std::endl;
	CheckCdrom();
  std::cout << "Loading CD-ROM" << std::endl;
  if (LoadCdrom() == -1)
  {
    std::cout << "Could not load CD-ROM" << std::endl;
  }
  std::cout << "Executing ..." << std::endl;
	psxCpu->Execute();
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
		setvbuf(emuLog, buf, _IOFBF, BUFSZ);
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

void SysUpdate() {
  /*
	PADhandleKey(PAD1_keypressed() );
	PADhandleKey(PAD2_keypressed() );

	SysDisableScreenSaver();
  */
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

static void ScanPlugins(gchar* scandir) {
	// scan for plugins and configuration tools
	DIR *dir;
	struct dirent *ent;

	gchar *linkname;
	gchar *filename;

	/* Any plugins found will be symlinked to the following directory */
	dir = opendir(scandir);
	if (dir != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			filename = g_build_filename (scandir, ent->d_name, NULL);

			if (match(filename, ".*\\.so$") == 0 &&
				match(filename, ".*\\.dylib$") == 0 &&
				match(filename, "cfg.*") == 0) {
				continue;	/* Skip this file */
			} else {
				/* Create a symlink from this file to the directory ~/.pcsxr/plugin */
				linkname = g_build_filename (getenv("HOME"), PLUGINS_DIR, ent->d_name, NULL);
				symlink(filename, linkname);

				/* If it's a config tool, make one in the cfg dir as well.
				   This allows plugins with retarded cfg finding to work :- ) */
				if (match(filename, "cfg.*") == 1) {
					linkname = g_build_filename (getenv("HOME"), PLUGINS_CFG_DIR, ent->d_name, NULL);
					symlink(filename, linkname);
				}
				g_free (linkname);
			}
			g_free (filename);
		}
		closedir(dir);
	}
}

static void ScanBios(gchar* scandir) {
	// scan for bioses
	DIR *dir;
	struct dirent *ent;

	gchar *linkname;
	gchar *filename;

	/* Any bioses found will be symlinked to the following directory */
	dir = opendir(scandir);
	if (dir != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			filename = g_build_filename(scandir, ent->d_name, NULL);

			if (match(filename, ".*\\.bin$") == 0 &&
				match(filename, ".*\\.BIN$") == 0) {
				continue;	/* Skip this file */
			} else {
				/* Create a symlink from this file to the directory ~/.pcsxr/plugin */
				linkname = g_build_filename(getenv("HOME"), BIOS_DIR, ent->d_name, NULL);
				symlink(filename, linkname);

				g_free(linkname);
			}
			g_free(filename);
		}
		closedir(dir);
	}
}

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

static void ScanAllPlugins (void) {
	gchar *currentdir;

	// scan some default locations to find plugins
	ScanPlugins("/usr/lib/games/psemu/");
	ScanPlugins("/usr/lib/games/psemu/lib/");
	ScanPlugins("/usr/lib/games/psemu/config/");
	ScanPlugins("/usr/local/lib/games/psemu/lib/");
	ScanPlugins("/usr/local/lib/games/psemu/config/");
	ScanPlugins("/usr/local/lib/games/psemu/");
	ScanPlugins("/usr/lib64/games/psemu/");
	ScanPlugins("/usr/lib64/games/psemu/lib/");
	ScanPlugins("/usr/lib64/games/psemu/config/");
	ScanPlugins("/usr/local/lib64/games/psemu/lib/");
	ScanPlugins("/usr/local/lib64/games/psemu/config/");
	ScanPlugins("/usr/local/lib64/games/psemu/");
	ScanPlugins("/usr/lib32/games/psemu/");
	ScanPlugins("/usr/lib32/games/psemu/lib/");
	ScanPlugins("/usr/lib32/games/psemu/config/");
	ScanPlugins("/usr/local/lib32/games/psemu/lib/");
	ScanPlugins("/usr/local/lib32/games/psemu/config/");
	ScanPlugins("/usr/local/lib32/games/psemu/");
	ScanPlugins(DEF_PLUGIN_DIR);
	ScanPlugins(DEF_PLUGIN_DIR "/lib");
	ScanPlugins(DEF_PLUGIN_DIR "/lib64");
	ScanPlugins(DEF_PLUGIN_DIR "/lib32");
	ScanPlugins(DEF_PLUGIN_DIR "/config");

	// scan some default locations to find bioses
	ScanBios("/usr/lib/games/psemu");
	ScanBios("/usr/lib/games/psemu/bios");
	ScanBios("/usr/lib64/games/psemu");
	ScanBios("/usr/lib64/games/psemu/bios");
	ScanBios("/usr/lib32/games/psemu");
	ScanBios("/usr/lib32/games/psemu/bios");
	ScanBios("/usr/share/psemu");
	ScanBios("/usr/share/psemu/bios");
	ScanBios("/usr/share/pcsxr");
	ScanBios("/usr/share/pcsxr/bios");
	ScanBios("/usr/local/lib/games/psemu");
	ScanBios("/usr/local/lib/games/psemu/bios");
	ScanBios("/usr/local/lib64/games/psemu");
	ScanBios("/usr/local/lib64/games/psemu/bios");
	ScanBios("/usr/local/lib32/games/psemu");
	ScanBios("/usr/local/lib32/games/psemu/bios");
	ScanBios("/usr/local/share/psemu");
	ScanBios("/usr/local/share/psemu/bios");
	ScanBios("/usr/local/share/pcsxr");
	ScanBios("/usr/local/share/pcsxr/bios");
	ScanBios(PSEMU_DATA_DIR);
	ScanBios(PSEMU_DATA_DIR "/bios");

	currentdir = g_strconcat(getenv("HOME"), "/.psemu-plugins/", NULL);
	ScanPlugins(currentdir);
	g_free(currentdir);

	currentdir = g_strconcat(getenv("HOME"), "/.psemu/", NULL);
	ScanPlugins(currentdir);
	g_free(currentdir);

	// Check for bad links in ~/.pcsxr/plugins/
	currentdir = g_build_filename(getenv("HOME"), PLUGINS_DIR, NULL);
	CheckSymlinksInPath(currentdir);
	g_free(currentdir);

	// Check for bad links in ~/.pcsxr/plugins/cfg
	currentdir = g_build_filename(getenv("HOME"), PLUGINS_CFG_DIR, NULL);
	CheckSymlinksInPath(currentdir);
	g_free(currentdir);

	// Check for bad links in ~/.pcsxr/bios
	currentdir = g_build_filename(getenv("HOME"), BIOS_DIR, NULL);
	CheckSymlinksInPath(currentdir);
	g_free(currentdir);
}
