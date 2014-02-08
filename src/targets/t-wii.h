 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Target specific stuff, *nix version
  *
  * Copyright 1997 Bernd Schmidt
  */

#define TARGET_NAME "wii"

#define TARGET_ROM_PATH         "/uae/roms"
#define TARGET_FLOPPY_PATH      "/uae/floppies"
#define TARGET_HARDFILE_PATH    "/uae/HD"
#define TARGET_SAVESTATE_PATH   "/uae/saves"
#define TARGET_SD_PATH   "sd:/"
#define TARGET_USB_PATH   "usb:/"
#define TARGET_SMB_PATH   "smb:/"

#ifndef OPTIONSFILENAME
# define OPTIONSFILENAME "/uae/uaerc"
#endif
#undef OPTIONS_IN_HOME
#define SMBFILENAME "/uae/uaerc.smb"
#define USERFILENAME "/uae/uaerc.user"
#define SAVEDFILENAME "/uae/uaerc.saved"
#define KBDIMAGE "/apps/uae/images/kb_amiga.png"

#define DEFPRTNAME "lpr"
#define DEFSERNAME "/dev/ttyS1"
