/*-
 * Copyright (c) 2023 Hans Petter Selasky
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <err.h>
#include <ctype.h>

#include <sys/stat.h>

static bool
encode_utf8(int ch, char *buffer, size_t *ppos, size_t size)
{
	char temp[4];
	size_t len;

	/* Convert to UTF-8. */

	if (ch < 0x80) {
		temp[0] = ch;
		len = 1;
	} else if (ch < 0x800) {
		temp[0] = 0xc0 | (ch >> 6);
		temp[1] = 0x80 | (ch & 0x3f);
		len = 2;
	} else if (ch < 0x10000) {
		temp[0] = 0xe0 | (ch >> 12);
		temp[1] = 0x80 | ((ch >> 6) & 0x3f);
		temp[2] = 0x80 | (ch & 0x3f);
		len = 3;
	} else {
		temp[0] = 0xf0 | (ch >> 18);
		temp[1] = 0x80 | ((ch >> 12) & 0x3f);
		temp[2] = 0x80 | ((ch >> 6) & 0x3f);
		temp[3] = 0x80 | (ch & 0x3f);
		len = 4;
	}

	/*
         * Make sure everything fits into the buffer.
         */
	if (*ppos + len > size)
		return (false);

	memcpy(buffer + *ppos, temp, len);
	*ppos += len;
	return (true);
}

static bool
skipchar(int ch)
{
	return (ch == '/' || ch == ':' || ch == '.' || ch == '\0' || (ch < 256 && isgraph(ch) == false));
}

static size_t success;

static void
create(const char *ptr)
{
	printf("mkdir %s\n", ptr);
	if (mkdir(ptr, 0744) != 0)
		err(1, "Cannot create directory '%s'", ptr);
	success++;
}

#define	MAXCHAR 0x400

int
main(int argc, char **argv)
{
	size_t pos;
	size_t old;

	char buffer[128];

	create("testdir");

	for (int a = 0; a != MAXCHAR; a++) {
		if (skipchar(a))
			continue;
		snprintf(buffer, sizeof(buffer), "testdir/");
		pos = strlen(buffer);

		if (encode_utf8(a, buffer, &pos, sizeof(buffer) - 1) == false)
			errx(1, "Buffer is too small");

		if (pos >= sizeof(buffer) - 2)
			errx(1, "Buffer is too small");

		buffer[pos] = 0;

		create(buffer);

		buffer[pos++] = '/';

		old = pos;

		for (int b = a + 1; b != MAXCHAR; b++) {

			if (skipchar(b))
				continue;

			pos = old;

			if (encode_utf8(a, buffer, &pos, sizeof(buffer) - 1) == false)
				errx(1, "Buffer is too small");

			if (encode_utf8(b, buffer, &pos, sizeof(buffer) - 1) == false)
				errx(1, "Buffer is too small");

			buffer[pos] = 0;

			create(buffer);
		}
	}

	snprintf(buffer, sizeof(buffer), "testdir/a");
	pos = strlen(buffer);

	for (int x = 0; x != 16; x++) {
		if (encode_utf8(0x30A, buffer, &pos, sizeof(buffer) - 1) == false)
			errx(1, "Buffer is too small");
	}

	buffer[pos] = 0;

	create(buffer);

	printf("Number of directories created: %zu\n", success);
	return (0);
}
