/*
	Audio Overload SDK - main driver.  for demonstration only, not user friendly!

	Copyright (c) 2007-2009 R. Belmont and Richard Bannister.

	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	* Neither the names of R. Belmont and Richard Bannister nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ao.h"
#include "eng_protos.h"

static struct 
{ 
	uint32 sig; 
	char *name; 
	void * (*start)(const char *path, uint8 *, uint32); // returns handle
	int32 (*gen)(void *handle, int16 *, uint32); 
	int32 (*stop)(void *handle); 
	int32 (*command)(void *handle, int32, int32); 
	uint32 rate; 
	int32 (*fillinfo)(void *handle, ao_display_info *); 
} types[] = {
	{ 0x50534641, "Capcom QSound (.qsf)", qsf_start, qsf_gen, qsf_stop, qsf_command, 60, qsf_fill_info },
	{ 0x50534611, "Sega Saturn (.ssf)", ssf_start, ssf_gen, ssf_stop, ssf_command, 60, ssf_fill_info },
	{ 0x50534601, "Sony PlayStation (.psf)", psf_start, psf_gen, psf_stop, psf_command, 60, psf_fill_info },
	{ 0x53505500, "Sony PlayStation (.spu)", spu_start, spu_gen, spu_stop, spu_command, 60, spu_fill_info },
	{ 0x50534602, "Sony PlayStation 2 (.psf2)", psf2_start, psf2_gen, psf2_stop, psf2_command, 60, psf2_fill_info },
	{ 0x50534612, "Sega Dreamcast (.dsf)", dsf_start, dsf_gen, dsf_stop, dsf_command, 60, dsf_fill_info },

	{ 0xffffffff, "", NULL, NULL, NULL, NULL, 0, NULL }
};

/* ao_get_lib: called to load secondary files */
int ao_get_lib(char *filename, uint8 **buffer, uint64 *length)
{
	uint8 *filebuf;
	uint32 size;
    FILE *auxfile;

        auxfile = fopen(filename, "rb");
	if (!auxfile)
	{
		fprintf(stderr, "Unable to find auxiliary file %s\n", filename);
		return AO_FAIL;
	}

        fseek(auxfile, 0, SEEK_END);
        size = ftell(auxfile);
        fseek(auxfile, 0, SEEK_SET);

	filebuf = malloc(size);

	if (!filebuf)
	{
        fclose(auxfile);
		printf("ERROR: could not allocate %d bytes of memory\n", size);
		return AO_FAIL;
	}

        fread(filebuf, size, 1, auxfile);
        fclose(auxfile);

	*buffer = filebuf;
	*length = (uint64)size;

	return AO_SUCCESS;
}

// stub for MAME stuff
int change_pc(int foo)
{
    return 0;
}

int
ao_identify (char *buffer) {
    uint32 filesig;
    uint32 type = 0;

	filesig = buffer[0]<<24 | buffer[1]<<16 | buffer[2]<<8 | buffer[3];
	while (types[type].sig != 0xffffffff)
	{
		if (filesig == types[type].sig)
		{
			break;
		}
		else
		{
			type++;
		}
	}

	// now did we identify it above or just fall through?
	if (types[type].sig != 0xffffffff)
	{
        printf ("psf: File identified as %s\n", types[type].name);
	}
	else
	{
        printf ("psf: File is unknown, signature bytes are %02x %02x %02x %02x\n", buffer[0], buffer[1], buffer[2], buffer[3]);
		return -1;
    }
    return type;
}

void *
ao_start (uint32 type, const char *path, uint8 *buffer, uint32 size) {
	return (*types[type].start)(path, buffer, size);
}

int
ao_stop (uint32 type, void *handle) {
	return (*types[type].stop)(handle);
}

int
ao_get_info (uint32 type, void *handle, ao_display_info *info) {
	return (*types[type].fillinfo)(handle, info);
}

int
ao_decode (uint32 type, void *handle, int16 *buffer, uint32 size) {
	(*types[type].gen)(handle, buffer, size);
	return size;
}

int
ao_command (uint32 type, void *handle, int32 command, int32 param) {
	return (*types[type].command)(handle, command, param);
}

void
ao_getlibpath (const char *path, const char *libname, char *libpath) {
    const char *e = strrchr (path, 0x5C); // '\'
    if (!e) {
        e = strrchr (path, 0x2F); // '/'
    }
    if (e) {
        e++;
        memcpy (libpath, path, e-path);
        libpath[e-path] = 0;
        strcat (libpath, libname);
    }
    else {
        strcpy (libpath, libname);
    }
}

