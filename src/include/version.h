/*
 * E-UAE - The portable Amiga Emulator
 *
 * Version/revision info.
 *
 * (c) 2006 Richard Drummond
 */

#ifndef EUAE_VERSION_H
#define EUAE_VERSION_H

#include "sysconfig.h"

#define UAEMAJOR   UAE_VERSION_MAJOR
#define UAEMINOR   UAE_VERSION_MINOR
#define UAESUBREV  UAE_VERSION_REVISION

#define UAEVERSION (256*65536L*UAEMAJOR + 65536L*UAEMINOR + UAESUBREV)

#ifdef PACKAGE_NAME
# define UAE_NAME PACKAGE_NAME
#else
# define UAE_NAME "E-UAE"
#endif

#define UAE_VERSION_STRING UAE_NAME " " UAE_VERSION

#endif
