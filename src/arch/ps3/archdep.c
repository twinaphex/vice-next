/*
 * archdep.c - Miscellaneous system-specific stuff.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <cell/hash/libmd5.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>

#ifdef HAVE_VFORK_H
#include <vfork.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "archdep.h"
#include "findpath.h"
#include "ioutil.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "ui.h"
#include "util.h"

#include "common.h"

static char *argv0 = NULL;
static char *boot_path = NULL;

/* alternate storage of preferences */
//const char *archdep_pref_path = NULL; /* NULL -> use home_path + ".vice" */
const char *archdep_pref_path = NULL; /* NULL -> use home_path + "VICEDIR" */

int archdep_network_init(void)
{
    return 0;
}

void archdep_network_shutdown(void)
{
}

int archdep_init(int *argc, char **argv)
{
	argv0 = lib_stralloc(argv[0]);
	log_verbose_init(*argc, argv);
	archdep_ui_init(*argc, argv);
	return 0;
}

char *archdep_program_name(void)
{
	static char *program_name = NULL;

	if (program_name == NULL) {
		char *p;

		p = strrchr(argv0, '/');
		if (p == NULL) {
			program_name = lib_stralloc(argv0);
		} else {
			program_name = lib_stralloc(p + 1);
		}
	}

	return program_name;
}

char *archdep_default_sysfile_pathlist(const char *emu_id)
{
	static char *default_path;

	if (default_path == NULL) {

		default_path = util_concat(
				//                                   VICE_USRDIR, ".vice/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
				//                                   VICE_USRDIR, ".vice/", "DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
				//                                   VICE_USRDIR, ".vice/", "PRINTER", ARCHDEP_FINDPATH_SEPARATOR_STRING,
				//                                   NULL);

			VICE_USRDIR, "VICEDIR/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
			VICE_USRDIR, "VICEDIR/", "DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
			VICE_USRDIR, "VICEDIR/", "PRINTER", ARCHDEP_FINDPATH_SEPARATOR_STRING,
			NULL);
	}

	return default_path;
}

/* Return a malloc'ed backup file name for file `fname'.  */
char *archdep_make_backup_filename(const char *fname)
{
	return util_concat(fname, "~", NULL);
}

char *archdep_default_resource_file_name(void)
{
	return util_concat(VICE_USRDIR, "/vicerc", NULL);
}

char *archdep_default_fliplist_file_name(void)
{
	return util_concat(VICE_USRDIR, "/fliplist-", machine_get_name(), ".vfl", NULL);
}

char *archdep_default_autostart_disk_image_file_name(void)
{
	return util_concat(VICE_USRDIR, "/autostart-", machine_get_name(), ".d64", NULL);
}

char *archdep_default_save_resource_file_name(void)
{ 
	return util_concat(VICE_USRDIR, "/vicerc", NULL);
}

FILE *archdep_open_default_log_file(void)
{
	return stdout;
}

int archdep_num_text_lines(void)
{
	return 0;
}

int archdep_num_text_columns(void)
{
	return 0;
}

int archdep_default_logger(const char *level_string, const char *txt)
{
	debug_printf ("%s%s\n", level_string, txt);
	return 0;
}

int archdep_path_is_relative(const char *path)
{
	if ( (path == NULL) || (strlen(path) == 0))
		return 0;

	return *path != '/';
}

int archdep_spawn(const char *name, char **argv, char **pstdout_redir, const char *stderr_redir)
{
	debug_printf ("ARCHDEP_SPAWN NOT IMPLEMENTED");
	return -1;
}

/* return malloc'd version of full pathname of orig_name */
int archdep_expand_path(char **return_path, const char *orig_name)
{
	/* Unix version.  */
	if (*orig_name == '/') {
		*return_path = lib_stralloc(orig_name);
	} else {
		static char *cwd;

		cwd = ioutil_current_dir();
		*return_path = util_concat(cwd, "/", orig_name, NULL);
		lib_free(cwd);
	}
	return 0;
}

void archdep_startup_log_error(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);

	debug_printf_quick (format, ap);
}

char *archdep_filename_parameter(const char *name)
{
	/* nothing special(?) */
	return lib_stralloc(name);
}

/*
    "special" chars in *unix are:

    "'\[]()´

    tested unproblematic (no escaping):

    "'()´

    tested problematic (need escaping):

    \[]
    - if the name of a file _inside_ a .zip file contain \, [ or ], then extracting
      it will fail if they are not escaped.

    several problems on autostart remain, which are not quoting but ascii vs petscii related.
*/
char *archdep_quote_parameter(const char *name)
{
	char *a;

	a = util_subst(name, "\\", "\\\\");
	a = util_subst(a, "[", "\\[");
	a = util_subst(a, "]", "\\]");
	return a;
}

char *tmpnam (char *s)
{
	static unsigned long tmpcount;
	static char buf[512];

	if (s == NULL)
		s = buf;
	(void)snprintf(s, 512, "tmp.%lu.%ul", tmpcount, sys_time_get_system_time() );
	++tmpcount;
	return s;
}

char *archdep_tmpnam(void)
{
	return util_concat(VICE_TMPDIR, tmpnam(NULL), NULL);
}

FILE *archdep_mkstemp_fd(char **filename, const char *mode)
{
	char *tmp;
	FILE *fd;

	static unsigned int len;
	static unsigned int isdir;

	if (archdep_stat(VICE_TMPDIR, &len, &isdir) != 0) {
		mkdir(VICE_TMPDIR, 0755);
	}

	tmp = util_concat(VICE_TMPDIR, tmpnam(NULL), NULL);
	fd = fopen(tmp, mode);

	if (fd == NULL) {
		return NULL;
	}

	*filename = tmp;

	return fd;
}

int archdep_file_is_gzip(const char *name)
{
    size_t l = strlen(name);

    if ((l < 4 || strcasecmp(name + l - 3, ".gz")) && (l < 3 || strcasecmp(name + l - 2, ".z")) && (l < 4 || toupper(name[l - 1]) != 'Z' || name[l - 4] != '.')) {
        return 0;
    }
    return 1;
}

int archdep_file_set_gzip(const char *name)
{
	return 0;
}

int archdep_mkdir(const char *pathname, int mode)
{
	return mkdir(pathname, (mode_t)mode);
}

#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
int archdep_stat(const char *file_name, unsigned int *len, unsigned int *isdir)
{
	struct stat statbuf;

	if (stat(file_name, &statbuf) < 0) {
		*len = 0;
		*isdir = 0;
		return -1;
	}


	*len = statbuf.st_size;
	*isdir = S_ISDIR(statbuf.st_mode);

	return 0;
}

/* set permissions of given file to rw, respecting current umask */
int archdep_fix_permissions(const char *file_name)
{
	return 0;
}


void archdep_shutdown(void)
{
	lib_free(argv0);
	lib_free(boot_path);
}

/* some PS3 specific replacement functions */

int access(const char *path, int mode)
{
	// Assume we can always access the file in all modes, if we can open it in read mode

	FILE *fd;
	fd = fopen (path, "r");

	if (fd != NULL) {
		fclose(fd);
		return 0;
	}
	return -1;
}

int chdir(const char *path)
{
	return 0;
}

extern void Emulator_Shutdown(void);

void main_exit(void)
{
	debug_printf ("machine_shutdown called from main_exit()\n");
	machine_shutdown();
	Emulator_Shutdown();
}

// TODO  No need for a 16 MB buffer. 
#define FILEBUF_SIZE            16*1024*1024
static unsigned char g_buf[FILEBUF_SIZE];
static char digest_string[CELL_MD5_DIGEST_SIZE*2+1];

char * md5_sum_file(const char *filepath)
{
	int fd;
	uint64_t  nread;
	int res;
	CellMd5WorkArea ctx;
	unsigned char digest[CELL_MD5_DIGEST_SIZE];

	res=cellFsOpen(filepath, CELL_FS_O_RDONLY, &fd, NULL, 0);

	if (fd < 0) {
		debug_printf("cellFsOpen('%s') error: 0x%08X\n", filepath, res);
		return NULL;
	}

	cellMd5BlockInit(&ctx);
	do {
		res = cellFsRead(fd, g_buf, sizeof(g_buf), &nread);
		if (res < 0) {
			debug_printf("cellFsRead('%s') error: 0x%08X\n", filepath, res);
			cellFsClose(fd);    
			return NULL;
		}
		debug_printf("read in %lld bytes from file\n", nread);
		debug_printf("Calculating MD5 chunk...\n");
		cellMd5BlockUpdate(&ctx, g_buf, (unsigned int)nread);
	} while (res == sizeof(g_buf));
	cellMd5BlockResult(&ctx, digest);
	cellFsClose(fd);

	{
		int i;

		debug_printf("\nMD5 Hash calculated from %s:\n", filepath);
		char *digest_ptr = digest_string;
		for(i = 0; i < CELL_MD5_DIGEST_SIZE; i++) {
			snprintf (digest_ptr, 2+1, "%02x", digest[i]);
			digest_ptr += 2;
		}
	}

	return digest_string;
}
