/*
 Author: José Bollo <jobol@nonadev.net>
 Author: José Bollo <jose.bollo@iot.bzh>

 https://gitlab.com/jobol/mustach

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "mustach.h"

#if defined(NO_EXTENSION_FOR_MUSTACH)
# undef  NO_COLON_EXTENSION_FOR_MUSTACH
# define NO_COLON_EXTENSION_FOR_MUSTACH
# undef  NO_ALLOW_EMPTY_TAG
# define NO_ALLOW_EMPTY_TAG
#endif

struct iwrap {
	int (*emit)(void *closure, const char *buffer, size_t size, int escape, FILE *file);
	void *closure; /* closure for: enter, next, leave, emit, get */
	int (*put)(void *closure, const char *name, int escape, FILE *file);
	void *closure_put; /* closure for put */
	int (*enter)(void *closure, const char *name);
	int (*next)(void *closure);
	int (*leave)(void *closure);
	int (*get)(void *closure, const char *name, struct mustach_sbuf *sbuf);
	int (*partial)(void *closure, const char *name, struct mustach_sbuf *sbuf);
	void *closure_partial; /* closure for partial */
};

#if !defined(NO_OPEN_MEMSTREAM)
static FILE *memfile_open(char **buffer, size_t *size)
{
	return open_memstream(buffer, size);
}
static void memfile_abort(FILE *file, char **buffer, size_t *size)
{
	fclose(file);
	free(*buffer);
	*buffer = NULL;
	*size = 0;
}
static int memfile_close(FILE *file, char **buffer, size_t *size)
{
	int rc;

	/* adds terminating null */
	rc = fputc(0, file) ? MUSTACH_ERROR_SYSTEM : 0;
	fclose(file);
	if (rc == 0)
		/* removes terminating null of the length */
		(*size)--;
	else {
		free(*buffer);
		*buffer = NULL;
		*size = 0;
	}
	return rc;
}
#else
static FILE *memfile_open(char **buffer, size_t *size)
{
	return tmpfile();
}
static void memfile_abort(FILE *file, char **buffer, size_t *size)
{
	fclose(file);
	*buffer = NULL;
	*size = 0;
}
static int memfile_close(FILE *file, char **buffer, size_t *size)
{
	int rc;
	size_t s;
	char *b;

	s = (size_t)ftell(file);
	b = malloc(s + 1);
	if (b == NULL) {
		rc = MUSTACH_ERROR_SYSTEM;
		errno = ENOMEM;
		s = 0;
	} else {
		rewind(file);
		if (1 == fread(b, s, 1, file)) {
			rc = 0;
			b[s] = 0;
		} else {
			rc = MUSTACH_ERROR_SYSTEM;
			free(b);
			b = NULL;
			s = 0;
		}
	}
	*buffer = b;
	*size = s;
	return rc;
}
#endif

static inline void sbuf_reset(struct mustach_sbuf *sbuf)
{
	sbuf->value = NULL;
	sbuf->freecb = NULL;
	sbuf->closure = NULL;
}

static inline void sbuf_release(struct mustach_sbuf *sbuf)
{
	if (sbuf->releasecb)
		sbuf->releasecb(sbuf->value, sbuf->closure);
}

static int iwrap_emit(void *closure, const char *buffer, size_t size, int escape, FILE *file)
{
	size_t i, j;

	(void)closure; /* unused */

	if (!escape)
		return fwrite(buffer, size, 1, file) != 1 ? MUSTACH_ERROR_SYSTEM : MUSTACH_OK;

	i = 0;
	while (i < size) {
		j = i;
		while (j < size && buffer[j] != '<' && buffer[j] != '>' && buffer[j] != '&')
			j++;
		if (j != i && fwrite(&buffer[i], j - i, 1, file) != 1)
			return MUSTACH_ERROR_SYSTEM;
		if (j < size) {
			switch(buffer[j++]) {
			case '<':
				if (fwrite("&lt;", 4, 1, file) != 1)
					return MUSTACH_ERROR_SYSTEM;
				break;
			case '>':
				if (fwrite("&gt;", 4, 1, file) != 1)
					return MUSTACH_ERROR_SYSTEM;
				break;
			case '&':
				if (fwrite("&amp;", 5, 1, file) != 1)
					return MUSTACH_ERROR_SYSTEM;
				break;
			default: break;
			}
		}
		i = j;
	}
	return MUSTACH_OK;
}

static int iwrap_put(void *closure, const char *name, int escape, FILE *file)
{
	struct iwrap *iwrap = closure;
	int rc;
	struct mustach_sbuf sbuf;
	size_t length;

	sbuf_reset(&sbuf);
	rc = iwrap->get(iwrap->closure, name, &sbuf);
	if (rc >= 0) {
		length = strlen(sbuf.value);
		if (length)
			rc = iwrap->emit(iwrap->closure, sbuf.value, length, escape, file);
		sbuf_release(&sbuf);
	}
	return rc;
}

static int iwrap_partial(void *closure, const char *name, struct mustach_sbuf *sbuf)
{
	struct iwrap *iwrap = closure;
	int rc;
	FILE *file;
	size_t size;
	char *result;

	result = NULL;
	file = memfile_open(&result, &size);
	if (file == NULL)
		rc = MUSTACH_ERROR_SYSTEM;
	else {
		rc = iwrap->put(iwrap->closure_put, name, 0, file);
		if (rc < 0)
			memfile_abort(file, &result, &size);
		else {
			rc = memfile_close(file, &result, &size);
			if (rc == 0) {
				sbuf->value = result;
				sbuf->freecb = free;
			}
		}
	}
	return rc;
}

static int process(const char *template, struct iwrap *iwrap, FILE *file, const char *opstr, const char *clstr)
{
	struct mustach_sbuf sbuf;
	char name[MUSTACH_MAX_LENGTH + 1], c, *tmp;
	const char *beg, *term;
	struct { const char *name, *again; size_t length; int enabled, entered; } stack[MUSTACH_MAX_DEPTH];
	size_t oplen, cllen, len, l;
	int depth, rc, enabled;

	enabled = 1;
	oplen = strlen(opstr);
	cllen = strlen(clstr);
	depth = 0;
	for(;;) {
		beg = strstr(template, opstr);
		if (beg == NULL) {
			/* no more mustach */
			if (enabled && template[0]) {
				rc = iwrap->emit(iwrap->closure, template, strlen(template), 0, file);
				if (rc < 0)
					return rc;
			}
			return depth ? MUSTACH_ERROR_UNEXPECTED_END : MUSTACH_OK;
		}
		if (enabled && beg != template) {
			rc = iwrap->emit(iwrap->closure, template, (size_t)(beg - template), 0, file);
			if (rc < 0)
				return rc;
		}
		beg += oplen;
		term = strstr(beg, clstr);
		if (term == NULL)
			return MUSTACH_ERROR_UNEXPECTED_END;
		template = term + cllen;
		len = (size_t)(term - beg);
		c = *beg;
		switch(c) {
		case '!':
		case '=':
			break;
		case '{':
			for (l = 0 ; clstr[l] == '}' ; l++);
			if (clstr[l]) {
				if (!len || beg[len-1] != '}')
					return MUSTACH_ERROR_BAD_UNESCAPE_TAG;
				len--;
			} else {
				if (term[l] != '}')
					return MUSTACH_ERROR_BAD_UNESCAPE_TAG;
				template++;
			}
			c = '&';
			/*@fallthrough@*/
		case '^':
		case '#':
		case '/':
		case '&':
		case '>':
#if !defined(NO_COLON_EXTENSION_FOR_MUSTACH)
		case ':':
#endif
			beg++; len--;
		default:
			while (len && isspace(beg[0])) { beg++; len--; }
			while (len && isspace(beg[len-1])) len--;
#if !defined(NO_ALLOW_EMPTY_TAG)
			if (len == 0)
				return MUSTACH_ERROR_EMPTY_TAG;
#endif
			if (len > MUSTACH_MAX_LENGTH)
				return MUSTACH_ERROR_TAG_TOO_LONG;
			memcpy(name, beg, len);
			name[len] = 0;
			break;
		}
		switch(c) {
		case '!':
			/* comment */
			/* nothing to do */
			break;
		case '=':
			/* defines separators */
			if (len < 5 || beg[len - 1] != '=')
				return MUSTACH_ERROR_BAD_SEPARATORS;
			beg++;
			len -= 2;
			for (l = 0; l < len && !isspace(beg[l]) ; l++);
			if (l == len)
				return MUSTACH_ERROR_BAD_SEPARATORS;
			oplen = l;
			tmp = alloca(oplen + 1);
			memcpy(tmp, beg, oplen);
			tmp[oplen] = 0;
			opstr = tmp;
			while (l < len && isspace(beg[l])) l++;
			if (l == len)
				return MUSTACH_ERROR_BAD_SEPARATORS;
			cllen = len - l;
			tmp = alloca(cllen + 1);
			memcpy(tmp, beg + l, cllen);
			tmp[cllen] = 0;
			clstr = tmp;
			break;
		case '^':
		case '#':
			/* begin section */
			if (depth == MUSTACH_MAX_DEPTH)
				return MUSTACH_ERROR_TOO_DEEP;
			rc = enabled;
			if (rc) {
				rc = iwrap->enter(iwrap->closure, name);
				if (rc < 0)
					return rc;
			}
			stack[depth].name = beg;
			stack[depth].again = template;
			stack[depth].length = len;
			stack[depth].enabled = enabled;
			stack[depth].entered = rc;
			if ((c == '#') == (rc == 0))
				enabled = 0;
			depth++;
			break;
		case '/':
			/* end section */
			if (depth-- == 0 || len != stack[depth].length || memcmp(stack[depth].name, name, len))
				return MUSTACH_ERROR_CLOSING;
			rc = enabled && stack[depth].entered ? iwrap->next(iwrap->closure) : 0;
			if (rc < 0)
				return rc;
			if (rc) {
				template = stack[depth++].again;
			} else {
				enabled = stack[depth].enabled;
				if (enabled && stack[depth].entered)
					iwrap->leave(iwrap->closure);
			}
			break;
		case '>':
			/* partials */
			if (enabled) {
				sbuf_reset(&sbuf);
				rc = iwrap->partial(iwrap->closure_partial, name, &sbuf);
				if (rc >= 0) {
					rc = process(sbuf.value, iwrap, file, opstr, clstr);
					sbuf_release(&sbuf);
				}
				if (rc < 0)
					return rc;
			}
			break;
		default:
			/* replacement */
			if (enabled) {
				rc = iwrap->put(iwrap->closure_put, name, c != '&', file);
				if (rc < 0)
					return rc;
			}
			break;
		}
	}
}

int fmustach(const char *template, struct mustach_itf *itf, void *closure, FILE *file)
{
	int rc;
	struct iwrap iwrap;

	/* check validity */
	if (!itf->enter || !itf->next || !itf->leave || (!itf->put && !itf->get))
		return MUSTACH_ERROR_INVALID_ITF;

	/* init wrap structure */
	iwrap.closure = closure;
	if (itf->put) {
		iwrap.put = itf->put;
		iwrap.closure_put = closure;
	} else {
		iwrap.put = iwrap_put;
		iwrap.closure_put = &iwrap;
	}
	if (itf->partial) {
		iwrap.partial = itf->partial;
		iwrap.closure_partial = closure;
	} else if (itf->get) {
		iwrap.partial = itf->get;
		iwrap.closure_partial = closure;
	} else {
		iwrap.partial = iwrap_partial;
		iwrap.closure_partial = &iwrap;
	}
	iwrap.emit = itf->emit ?: iwrap_emit;
	iwrap.enter = itf->enter;
	iwrap.next = itf->next;
	iwrap.leave = itf->leave;
	iwrap.get = itf->get;

	/* process */
	rc = itf->start ? itf->start(closure) : 0;
	if (rc == 0)
		rc = process(template, &iwrap, file, "{{", "}}");
	if (itf->stop)
		itf->stop(closure, rc);
	return rc;
}

int fdmustach(const char *template, struct mustach_itf *itf, void *closure, int fd)
{
	int rc;
	FILE *file;

	file = fdopen(fd, "w");
	if (file == NULL) {
		rc = MUSTACH_ERROR_SYSTEM;
		errno = ENOMEM;
	} else {
		rc = fmustach(template, itf, closure, file);
		fclose(file);
	}
	return rc;
}

int mustach(const char *template, struct mustach_itf *itf, void *closure, char **result, size_t *size)
{
	int rc;
	FILE *file;
	size_t s;

	*result = NULL;
	if (size == NULL)
		size = &s;
	file = memfile_open(result, size);
	if (file == NULL)
		rc = MUSTACH_ERROR_SYSTEM;
	else {
		rc = fmustach(template, itf, closure, file);
		if (rc < 0)
			memfile_abort(file, result, size);
		else
			rc = memfile_close(file, result, size);
	}
	return rc;
}

