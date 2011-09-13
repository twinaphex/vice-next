/******************************************************************************* 
 *  -- ui_snapshot.cpp - Snapshot interface
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *
 *  Copyright (C) 2010
 *  Created on: Dec 23, 2010
 *  Author:      TimRex
 *
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ********************************************************************************/

#include <string>
#include "ui_snapshot.h"
#include "ps3debug.h"

extern "C" {
#include "common.h"
#include "machine.h"
#include "ioutil.h"
#include "util.h"
#include "lib.h"
#include "log.h"
#include "lib/zlib/zlib.h"
#include "archdep.h"
}

// Check for the existence of a snapshot matching this romfile
bool snapshot_exists(const char *filepath)
{
	// Get MD5 of 'current_rom'

	static const char *md5filename=NULL;
	char *digest;
	digest = md5_sum_file(filepath);

	if (digest) {
		if (md5filename)
			lib_free(md5filename);

		md5filename = util_concat (VICE_SNAPSHOT_DIR, digest, ".vsf", NULL);
	}

	return (ioutil_access(md5filename, 'r') == 0);
}

// load a snapshot matching this romfile
int load_snapshot(const char *filepath)
{
	const char *md5filename;
	const char *vsffilename;

	if (filepath == NULL)
		return -1;


	// Read the snapshot from a hash lookup file of the currently running image file
	// Get MD5 of 'current_rom'

	char *digest;
	digest = md5_sum_file(filepath);

	if (digest) {
		md5filename = util_concat (VICE_SNAPSHOT_DIR, digest, NULL);

		// load snapshot to VICE_SNAPSHOT_DIR
		debug_printf ("Reading snapshot from %s for %s\n", md5filename, filepath);

		vsffilename = util_concat(md5filename, ".vsf", NULL);

		if (machine_read_snapshot(vsffilename, 0) != 0) {
			debug_printf ("Failed to read snapshot from file %s\n", vsffilename);
		}

		lib_free(md5filename);
		return 0;
	}
	return -1;
}


// save a snapshot matching this romfile
int save_snapshot(const char *filepath)
{
	const char *md5filename;
	const char *vsffilename;

	FILE *fdout;
	static unsigned int len;
	static unsigned int isdir;


	if (filepath == NULL)
		return -1;

	if (archdep_stat(VICE_SNAPSHOT_DIR, &len, &isdir) != 0)
	{
		debug_printf ("archdep_mkstemp_fd creating VICE_SNAPSHOT_DIR : %s\n", VICE_SNAPSHOT_DIR);
		archdep_mkdir(VICE_SNAPSHOT_DIR, 0755);
	}

	// Write the snapshot to a hash lookup file of the currently running image file
	// Get MD5 of 'current_rom'

	char *digest;
	digest = md5_sum_file(filepath);

	if (digest)
	{
		md5filename = util_concat (VICE_SNAPSHOT_DIR, digest, NULL);

		// save snapshot to VICE_SNAPSHOT_DIR
		debug_printf ("Writing snapshot to %s.vsf for %s\n", md5filename, filepath);
		vsffilename = util_concat(md5filename, ".vsf", NULL);

		if (machine_write_snapshot(vsffilename, 1, 1, 0) != 0) {
			debug_printf ("Failed to write snapshot to file %s\n", vsffilename);
		}


		// Create metadata file
		const char *metafilename = util_concat(md5filename, ".txt", NULL);
		fdout = (FILE *) fopen(metafilename, "w");
		if (fdout == NULL) {
			log_error (LOG_DEFAULT, "Failed to open metafile for writing %s\n", metafilename);
			fclose(fdout);
			lib_free(md5filename);
			lib_free(metafilename);
			return -1;
		}
		int bytes_wrote = fwrite (filepath, 1, strlen(filepath), fdout);
		if (bytes_wrote < (signed) strlen(filepath)) {
			fclose(fdout);
			log_error (LOG_DEFAULT, "Failed to write metafilename %s, wrote %d bytes of total %d.\n", metafilename, bytes_wrote, strlen(filepath));
			lib_free(md5filename);
			lib_free(metafilename);
			return -1;
		}
		fclose(fdout);
		lib_free(metafilename);
		return 0;
	}
	return -1;
}
