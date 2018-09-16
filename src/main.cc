#include <dlfcn.h>
#include <iostream>
#include <signal.h>
#include <string>

#include "../libpcsxcore/plugins.h"

const char ISO_FILENAME[] = "/hme/infcpl00/psx_games/tekken3.bin";

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

void SysReset() {
	EmuReset();
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
