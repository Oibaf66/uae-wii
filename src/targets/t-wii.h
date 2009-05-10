 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Target specific stuff, *nix version
  *
  * Copyright 1997 Bernd Schmidt
  */

#define TARGET_NAME "wii"

#define TARGET_ROM_PATH         "/apps/uae/roms"
#define TARGET_FLOPPY_PATH      "/apps/uae/floppies"
#define TARGET_HARDFILE_PATH    "/apps/uae/harddisks"
#define TARGET_SAVESTATE_PATH   "/apps/uae/saves"

#ifndef OPTIONSFILENAME
# define OPTIONSFILENAME "/apps/uae/uaerc"
#endif
#undef OPTIONS_IN_HOME

#define DEFPRTNAME "lpr"
#define DEFSERNAME "/dev/ttyS1"
