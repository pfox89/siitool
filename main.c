/* siitool
 *
 * Simple program to print the values of the SII EEPROM content. It reads the
 * raw binaries.
 *
 * Frank Jeschke <fjeschke@synapticon.de>
 */

/*
 *       Copyright (c) 2013, Synapticon GmbH
 *       All rights reserved.
 *
 *       Redistribution and use in source and binary forms, with or without
 *       modification, are permitted provided that the following conditions are met:
 *
 *       1. Redistributions of source code must retain the above copyright notice, this
 *          list of conditions and the following disclaimer.
 *       2. Redistributions in binary form must reproduce the above copyright notice,
 *          this list of conditions and the following disclaimer in the documentation
 *          and/or other materials provided with the distribution.
 *       3. Execution of this software or parts of it exclusively takes place on hardware
 *          produced by Synapticon GmbH.
 *
 *       THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *       ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *       WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *       DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 *       ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *       (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *       LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *       ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *       (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *       SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *       The views and conclusions contained in the software and documentation are those
 *       of the authors and should not be interpreted as representing official policies,
 *       either expressed or implied, of the Synapticon GmbH.
 */

#include "sii.h"
#include "esi.h"
#include "esifile.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define VERSION_MAJOR    0
#define VERSION_MINNOR   1

#define MAX_BUFFER_SIZE    (1000*1024)
#define MAX_FILENAME_SIZE  (256)

enum eInputFileType {
	UNDEFINED = 0
	,ESIXML
	,SIIEEPROM
};

//static int g_print_offsets = 0;

static const char *base(const char *prog)
{
	const char *p = prog;

	while (*p != '\0') {
		if (*p == '/')
			prog = ++p;
		else
			p++;
	}

	return prog;
}

static char *get_suffix(const char *filename)
{
	char *suffix = (char *)(filename+strlen(filename));

	while (suffix != filename) {
		if (*suffix == '.')
			return (suffix+1);

		suffix--;
	}

	return NULL; /* file doesn't have a valid suffix */
}

static enum eInputFileType check_file_suffix(const char *filename)
{
	enum eInputFileType type = UNDEFINED;

	char *suffix = get_suffix(filename);

	if (suffix == NULL)
		type = UNDEFINED;
	else {
		if (strncmp(suffix, "bin", strlen(suffix)) == 0)
			type = SIIEEPROM;
		else if (strncmp(suffix, "sii", strlen(suffix)) == 0)
			type = SIIEEPROM;
		else if (strncmp(suffix, "xml", strlen(suffix)) == 0)
			type = SIIEEPROM;
		else {
			fprintf(stderr, "Warning, unrecognized suffix: '%s'\n", suffix);
			type = UNDEFINED;
		}
	}

	return type;
}

static enum eInputFileType check_first_bytes(unsigned char *buffer)
{
	enum eInputFileType type = UNDEFINED;

	if (strncmp((char *)buffer, "<?xml", 5) == 0)
		type = ESIXML;
	else
		type = SIIEEPROM; /* educated guess: if it's not XML it must be SII */

	return type;
}

static enum eInputFileType file_type(const char *filename, unsigned char *buffer)
{
	enum eInputFileType inputtype1 = UNDEFINED;
	if (filename != NULL)
		inputtype1 = check_file_suffix(filename);

	enum eInputFileType inputtype2 = UNDEFINED;
	inputtype2 = check_first_bytes(buffer);

	if (inputtype1 == UNDEFINED)
		return inputtype2;

	if (inputtype1 != inputtype2) {
		fprintf(stderr, "Error, file suffix and content of file doesn't match!\n");
		return UNDEFINED;
	}

	return inputtype1;
}

static void printhelp(const char *prog)
{
	printf("Usage: %s [-h] [-v] [filename]\n", prog);
	printf("  -h         print this help and exit\n");
	printf("  -v         print version an exit\n");
	printf("  -o <name>  write output to file <name>\n");
	printf("  filename   path to eeprom file, if missing read from stdin\n");
	printf("\nRecognized file types: SII and ESI/XML.\n");
}

static int read_input(FILE *f, unsigned char *buffer, size_t size)
{
	size_t count = 0;
	int input;

	while ((input=fgetc(f)) != EOF)
		buffer[count++] = (unsigned char)(input&0xff);

	if (count>size)
		fprintf(stderr, "Error: read size is larger than expected (bytes read: %d)\n", count);

	return count;
}


int main(int argc, char *argv[])
{
	FILE *f;
	unsigned char eeprom[MAX_BUFFER_SIZE];
	const char *filename = NULL;
	char output[MAX_FILENAME_SIZE];

	for (int i=1; i<argc; i++) {
		switch (argv[i][0]) {
		case '-':
			if (argv[i][1] == 'h') {
				printhelp(base(argv[0]));
				return 0;
			} else if (argv[i][1] == 'v') {
				printf("Version %d.%d\n", VERSION_MAJOR, VERSION_MINNOR);
				return 0;
			} else if (argv[i][1] == 'o') {
				i++;
				strncpy(output, argv[i], strlen(argv[i]));
			} else if (argv[i][1] == '-') {
				filename = NULL;
			} else {
				fprintf(stderr, "Invalid argument\n");
				printhelp(base(argv[0]));
				return 0;
			}
			break;

		default:
			/* asuming file name */
			filename = argv[i];
			break;
		}
	}

	if (filename == NULL)
		read_input(stdin, eeprom, MAX_BUFFER_SIZE);
	else {
		f = fopen(filename, "r");
		if (f == NULL) {
			perror("Error open input file");
			return -1;
		}

		printf("Start reading contents of file %s\n", filename);

		read_input(f, eeprom, MAX_BUFFER_SIZE);
		fclose(f);
	}

	/* recognize input */
	enum eInputFileType filetype = file_type(filename, eeprom);
	switch (filetype) {
	case ESIXML:
		printf("Processing ESI/XML file\n");
		break;
	case SIIEEPROM:
		printf("Processing SII/EEPROM file\n");
		break;
	case UNDEFINED:
	default:
		return -1;
	}


	return 0; /* DEBUG command line arguments */

	EsiData *esi = esi_init_string(eeprom, MAX_BUFFER_SIZE);
	//esi_print_xml(esi);
	if (esi_parse(esi))
		fprintf(stderr, "Error something went wrong in XML parsing\n");

	esi_print_sii(esi);
	esi_release(esi);

	return 0;

	//return parse_and_print_content(eeprom, 1024);
	SiiInfo *sii = sii_init_string(eeprom, 1024);
	//alternative: SiiInfo *sii = sii_init_file(filename) */

	sii_print(sii);
	sii_generate(sii);
	int ret = sii_write_bin(sii, "siiout.bin");
	if (ret < 0)
		fprintf(stderr, "Error, couldn't write output file\n");

	sii_release(sii);

	return 0;
}
