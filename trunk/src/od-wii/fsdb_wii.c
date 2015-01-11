 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Library of functions to make emulated filesystem as independent as
  * possible of the host filesystem's capabilities.
  * This is the Win32 version.
  *
  * Copyright 1997 Mathias Ortmann
  * Copyright 1999 Bernd Schmidt
  */

#ifdef FILESYS

#include "sysconfig.h"
#include "sysdeps.h"

#include "fsdb.h"
#include <fat.h>

#define TRACING_ENABLED 0
#if TRACING_ENABLED
#define TRACE(x)	do { write_log x; } while(0)
#else
#define TRACE(x)
#endif

/* these are deadly (but I think allowed on the Amiga): */
#define NUM_EVILCHARS 7
static const char evilchars[NUM_EVILCHARS] = { '\\', '*', '?', '\"', '<', '>', '|' };

/* Return nonzero for any name we can't create on the native filesystem.  */
int fsdb_name_invalid (const char *n)
{
    int i;
    char a = n[0];
    char b = (a == '\0' ? a : n[1]);
    char c = (b == '\0' ? b : n[2]);
    char d = (c == '\0' ? c : n[3]);
    int l = strlen (n), ll;

    if (a >= 'a' && a <= 'z')
	a -= 32;
    if (b >= 'a' && b <= 'z')
	b -= 32;
    if (c >= 'a' && c <= 'z')
	c -= 32;

    /* reserved dos devices */
    ll = 0;
    if (a == 'A' && b == 'U' && c == 'X') ll = 3; /* AUX  */
    if (a == 'C' && b == 'O' && c == 'N') ll = 3; /* CON  */
    if (a == 'P' && b == 'R' && c == 'N') ll = 3; /* PRN  */
    if (a == 'N' && b == 'U' && c == 'L') ll = 3; /* NUL  */
    if (a == 'L' && b == 'P' && c == 'T'  && (d >= '0' && d <= '9')) ll = 4;  /* LPT# */
    if (a == 'C' && b == 'O' && c == 'M'  && (d >= '0' && d <= '9')) ll = 4; /* COM# */
    /* AUX.anything, CON.anything etc.. are also illegal names */
    if (ll && (l == ll || (l > ll && n[ll] == '.')))
	return 1;

    /* spaces and periods at the end are a no-no */
    i = l - 1;
    if (n[i] == '.' || n[i] == ' ')
	return 1;

    /* these characters are *never* allowed */
    for (i = 0; i < NUM_EVILCHARS; i++) {
	if (strchr (n, evilchars[i]) != 0)
	    return 1;
    }

    /* the reserved fsdb filename */
    if (strcmp (n, FSDB_FILE) == 0)
	return 1;
    return 0; /* the filename passed all checks, now it should be ok */
}

static uae_u32 filesys_parse_mask (uae_u32 mask)
{
    return mask ^ 0xf;
}

int fsdb_exists (char *nname)
{
    struct stat statbuf;

    return (stat (nname, &statbuf) != -1);
}

/* For an a_inode we have newly created based on a filename we found on the
 * native fs, fill in information about this file/directory.  */
int fsdb_fill_file_attrs (a_inode *base, a_inode *aino)
{
    int mode;

    if ((mode = FAT_getAttr (aino->nname)) <0) {
	write_log ("FAT_getAttr('%s') failed! error=%d, aino=%p dir=%d\n", aino->nname,errno,aino,aino->dir);
	return 0;
    }

    aino->dir = (mode & ATTR_DIRECTORY) ? 1 : 0;
    aino->amigaos_mode = A_FIBF_EXECUTE | A_FIBF_READ;
    if (ATTR_ARCHIVE & mode)
	aino->amigaos_mode |= A_FIBF_ARCHIVE;
    if (! (ATTR_READONLY & mode))
	aino->amigaos_mode |= A_FIBF_WRITE | A_FIBF_DELETE;
    aino->amigaos_mode = filesys_parse_mask (aino->amigaos_mode);
    return 1;
}

int fsdb_set_file_attrs (a_inode *aino)
{
    struct stat statbuf;
    int mode=0, tmpmask;

    tmpmask = filesys_parse_mask (aino->amigaos_mode);

    if (stat (aino->nname, &statbuf) == -1)
	return ERROR_OBJECT_NOT_AROUND;

    /* Unix dirs behave differently than AmigaOS ones.  */
    /* windows dirs go where no dir has gone before...  */
    if (! aino->dir) {
	if ((tmpmask & (A_FIBF_WRITE | A_FIBF_DELETE)) == 0)
	    mode |= ATTR_READONLY;
	if (tmpmask & A_FIBF_ARCHIVE)
	    mode |= ATTR_ARCHIVE;
	//else
	    //mode &= ~ATTR_ARCHIVE;

	FAT_setAttr(aino->nname, mode);
    }

    aino->dirty = 1;
    return 0;
}

/* Return nonzero if we can represent the amigaos_mode of AINO within the
 * native FS.  Return zero if that is not possible.  */
int fsdb_mode_representable_p (const a_inode *aino)
{
    int mask = aino->amigaos_mode;
    int m1;

    if (aino->dir)
	return aino->amigaos_mode == 0;

    /* P or S set, or E or R clear, means we can't handle it.  */
    if (mask & (A_FIBF_SCRIPT | A_FIBF_PURE | A_FIBF_EXECUTE | A_FIBF_READ))
	return 0;
    m1 = A_FIBF_DELETE | A_FIBF_WRITE;
    /* If it's rwed, we are OK... */
    if ((mask & m1) == 0) //Both delete and write protected
	return 1;
    /* We can also represent r-e-, by setting the host's readonly flag.  */
    if ((mask & m1) == m1) //Neither delete nor write protected
	return 1;
    return 0;
}

char *fsdb_create_unique_nname (a_inode *base, const char *suggestion)
{
    char *c;
    char tmp[256] = "__uae___";
    int i;

    strncat (tmp, suggestion, 240);

    /* replace the evil ones... */
    for (i=0; i < NUM_EVILCHARS; i++)
	while ((c = strchr (tmp, evilchars[i])) != 0)
	    *c = '_';

    while ((c = strchr (tmp, '.')) != 0)
	*c = '_';
    while ((c = strchr (tmp, ' ')) != 0)
	*c = '_';

    for (;;) {
	char *p = build_nname (base->nname, tmp);
	struct stat st;

	if (stat(p, &st) < 0) {
		write_log ("unique name: %s\n", p);
		return p;
	}
	free (p);

	/* tmpnam isn't reentrant and I don't really want to hack configure
	 * right now to see whether tmpnam_r is available...  */
	for (i = 0; i < 8; i++) {
	    tmp[i+8] = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[rand () % 63];
	}
    }
}

int dos_errno (void)
{
    int e = errno;

    switch (e) {
	case ENOMEM:	return ERROR_NO_FREE_STORE;
	case EEXIST:	return ERROR_OBJECT_EXISTS;
	case EACCES:	return ERROR_WRITE_PROTECTED;
	case ENOENT:	return ERROR_OBJECT_NOT_AROUND;
	case ENOTDIR:	return ERROR_OBJECT_WRONG_TYPE;
	case ENOSPC:	return ERROR_DISK_IS_FULL;
	case EBUSY:       	return ERROR_OBJECT_IN_USE;
	case EISDIR:	return ERROR_OBJECT_WRONG_TYPE;
#if defined(ETXTBSY)
	case ETXTBSY:	return ERROR_OBJECT_IN_USE;
#endif
#if defined(EROFS)
	case EROFS:       	return ERROR_DISK_WRITE_PROTECTED;
#endif
#if defined(ENOTEMPTY)
#if ENOTEMPTY != EEXIST
	case ENOTEMPTY:	return ERROR_DIRECTORY_NOT_EMPTY;
#endif
#endif
	default:
	TRACE (("FSDB: Unimplemented error: %s\n", strerror (e)));
	return ERROR_NOT_IMPLEMENTED;
    }
}
#endif
