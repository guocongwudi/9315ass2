/*
 * $PostgreSQL: pgsql/src/tutorial/complex.c,v 1.15 2009/06/11 14:49:15 momjian Exp $
 *
 ******************************************************************************
  This file contains routines that can be bound to a Postgres backend and
  called by the backend in the process of processing queries.  The calling
  format for these routines is dictated by Postgres architecture.
******************************************************************************/

#include  "postgres.h"

#include  "fmgr.h"
#include  "libpq/pqformat.h"		/* needed for send/recv functions */


PG_MODULE_MAGIC;

typedef struct parsed_url {
	char *scheme;
	char *host;
	char *port;
	char *path;
	char *params;
} Url;



char *str_n_dup(char *, int);
Url *parseURL(char *);
void printParsedURL(Url *);
Url *makeParsedURL();
Url *freeParsedURL(Url *);

char *str_n_dup(char *str, int n) {
	char *new = malloc(n + 1);
	if (new == NULL
	)
		return NULL;
	strncpy(new, str, n);
	new[n] = '\0';
	return new;
}

Url *parseURL(char *url) {
	char *c, *d;
	Url *purl = NULL;

	// trim trailing newline
	c = url;
	while (*c != '\0' && *c != '\n')
		c++;
	if (*c == '\n')
		*c = '\0';

	// create ParsedURL object
	if ((purl = makeParsedURL()) == NULL
	)
		return NULL;

	// start parse
	c = d = url;

	// find scheme component
	while (*d != '\0' && *d != ':')
		d++;
	// didn't find scheme
	if (*d == '\0')
		return freeParsedURL(purl);

	// copy scheme
	purl->scheme = str_n_dup(c, d - c);
	// must be "http" or "https"
	if (strcmp(purl->scheme, "http") != 0 && strcmp(purl->scheme, "https") != 0)
		return freeParsedURL(purl);

	// copy host
	if (*(d + 1) != '/' && *(d + 2) != '/')
		return freeParsedURL(purl);
	c = d = d + 3; // skip over '//'
	//skip other/
	while (*d == '/')
		d++;
	c = d;

	while (*d != '\0' && *d != ':' && *d != '/')
		d++;

	if (*d == '\0')
		return freeParsedURL(purl);
	purl->host = str_n_dup(c, d - c);
	// must contain at least one dot
	if (strchr(purl->host, '.') == NULL
	)
		return freeParsedURL(purl);

	// copy port, if any
	if (*d == ':') {
		c = d = d + 1; // skip over ':'
		while (*d != '\0' && *d != '/')
			d++;

		purl->port = str_n_dup(c, d - c);
	}
//	  else {
//		if (strcmp(purl->scheme, "http") == 0)
//			purl->port = "80";
//
//		else
//			purl->port = "403";
//
//	}
//	**************merge*****************//
	//	default port
	if (purl->port == NULL) {
		if (strcmp(purl->scheme, "http") == 0) {
			purl->port = malloc(3);
			strcpy(purl->port, "80");
			purl->port[2] = '\0';
		} else {
			purl->port = malloc(4);
			strcpy(purl->port, "443");
			purl->port[3] = '\0';
		}
	}

	//	if url end with / and final component is not path, treat it as invalid
	if (*d == '/' && *d + 1 == '\0')
		return freeParsedURL(purl);


// ****************merge************//


	// copy path, if any

	if (*d != '\0') {
		c = d = d + 1; // skip over '/'
		//skip other/
		while (*d == '/')
			d++;
		c = d;
		if (*d != '\0') {
			while (*d != '\0' && *d != '?')
				d++;
			purl->path = str_n_dup(c, d - c);
		}
	}
//	********************merge**********************//
	//	default path
		if(purl->path == NULL) {
			purl->path = malloc(11);
			strcpy(purl->path, "index.html");
			purl->path[10] = '\0';
		}
//		****************merge**********************//

	// copy params, if any
	if (*d != '\0') {
		c = d = d + 1; // skip over '?'
		if (*d != '\0') {
			purl->params = strdup(c);
		}
	}

	return purl;
}

Url *makeParsedURL() {
	Url *purl;
	if ((purl = malloc(sizeof(Url))) == NULL
	)
		return NULL;
	purl->scheme = NULL;
	purl->host = NULL;
	purl->port = NULL;
	purl->path = NULL;
	purl->params = NULL;
	return purl;
}

Url *freeParsedURL(Url *purl) {
	if (purl == NULL
	)
		return NULL;
	if (purl->scheme != NULL
	)
		free(purl->scheme);
	if (purl->host != NULL
	)
		free(purl->host);
	if (purl->port != NULL
	)
		free(purl->port);
	if (purl->path != NULL
	)
		free(purl->path);
	if (purl->params != NULL
	)
		free(purl->params);
	free(purl);
	return NULL;
}

















/*
 * Since we use V1 function calling convention, all these functions have
 * the same signature as far as C is concerned.  We provide these prototypes
 * just to forestall warnings when compiled with gcc -Wmissing-prototypes.
 */
Datum		url_in(PG_FUNCTION_ARGS);
Datum		url_out(PG_FUNCTION_ARGS);
Datum		url_recv(PG_FUNCTION_ARGS);
Datum		url_send(PG_FUNCTION_ARGS);
Datum		url_add(PG_FUNCTION_ARGS);
Datum		url_abs_lt(PG_FUNCTION_ARGS);
Datum		url_abs_le(PG_FUNCTION_ARGS);
Datum		url_abs_eq(PG_FUNCTION_ARGS);
Datum		url_abs_ge(PG_FUNCTION_ARGS);
Datum		url_abs_gt(PG_FUNCTION_ARGS);
Datum		url_abs_cmp(PG_FUNCTION_ARGS);


/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(url_in);

Datum
url_in(PG_FUNCTION_ARGS)
{	char	   *str = PG_GETARG_CSTRING(0);

	Url    *   url;
	url=parseURL(str);
	if (  url == NULL)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for Url: \"%s\"",
						str)));


	PG_RETURN_POINTER(url);
}

PG_FUNCTION_INFO_V1(url_out);

Datum
url_out(PG_FUNCTION_ARGS)
{
	Url    *url = (Url *) PG_GETARG_POINTER(0);
	char	   *result="";


	int url_len=0;
	url_len = strlen(url->host) +strlen(url->params) +strlen(url->path)+strlen(url->port)+strlen(url->scheme);


	result = (char *) palloc(url_len+5);
	result = strcat( result, url->scheme);
	result = strcat( result, "://");

	result = strcat( result, url->host);
	result = strcat( result,    ":");
	result = strcat( result,  url->port  );
	result = strcat( result,  "/"  );

	if(url->path!=NULL)
	{
		result = strcat( result, url->path  );
	}
	if(url->params!=NULL)
	{
		result = strcat( result, "?"  );
		result = strcat( result, url->params  );
	}


	PG_RETURN_CSTRING(result);
}

/*****************************************************************************
 * Binary Input/Output functions
 *
 * These are optional.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(url_recv);

Datum
url_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	Url    *result;

	result = (Url *) palloc(sizeof(Url));
	result->host = pq_getstring(buf);
	result->params = pq_getstring(buf);
	result->path = pq_getstring(buf);
	result->port = pq_getstring(buf);
	result->scheme = pq_getstring(buf);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(url_send);

Datum
url_send(PG_FUNCTION_ARGS)
{
	Url    *url = (Url *) PG_GETARG_POINTER(0);
	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendstring(&buf, url->host);
	pq_sendstring(&buf, url->params);
	pq_sendstring(&buf, url->path);
	pq_sendstring(&buf, url->port);
	pq_sendstring(&buf, url->scheme);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * New Operators
 *
 * A practical Url datatype would provide much more than this, of course.
 *****************************************************************************/



/*****************************************************************************
 * Operator class for defining B-tree index
 *
 * It's essential that the comparison operators and support function for a
 * B-tree index opclass always agree on the relative ordering of any two
 * data values.  Experience has shown that it's depressingly easy to write
 * unintentionally inconsistent functions.	One way to reduce the odds of
 * making a mistake is to make all the functions simple wrappers around
 * an internal three-way-comparison function, as we do here.
 *****************************************************************************/




PG_FUNCTION_INFO_V1(url_abs_lt);

Datum
url_abs_lt(PG_FUNCTION_ARGS)
{
	Url    *a = (Url *) PG_GETARG_POINTER(0);
	Url    *b = (Url *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(url_abs_cmp_internal(a, b) < 0);
}

PG_FUNCTION_INFO_V1(url_abs_le);

Datum
url_abs_le(PG_FUNCTION_ARGS)
{
	Url    *a = (Url *) PG_GETARG_POINTER(0);
	Url    *b = (Url *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(url_abs_cmp_internal(a, b) <= 0);
}

PG_FUNCTION_INFO_V1(url_abs_eq);

Datum
url_abs_eq(PG_FUNCTION_ARGS)
{
	Url    *a = (Url *) PG_GETARG_POINTER(0);
	Url    *b = (Url *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(url_abs_cmp_internal(a, b) == 0);
}

PG_FUNCTION_INFO_V1(url_abs_ge);

Datum
url_abs_ge(PG_FUNCTION_ARGS)
{
	Url    *a = (Url *) PG_GETARG_POINTER(0);
	Url    *b = (Url *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(url_abs_cmp_internal(a, b) >= 0);
}

PG_FUNCTION_INFO_V1(url_abs_gt);

Datum
url_abs_gt(PG_FUNCTION_ARGS)
{
	Url    *a = (Url *) PG_GETARG_POINTER(0);
	Url    *b = (Url *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(url_abs_cmp_internal(a, b) > 0);
}

PG_FUNCTION_INFO_V1(url_abs_cmp);

Datum
url_abs_cmp(PG_FUNCTION_ARGS)
{
	Url    *a = (Url *) PG_GETARG_POINTER(0);
	Url    *b = (Url *) PG_GETARG_POINTER(1);

	PG_RETURN_INT32(url_abs_cmp_internal(a, b));
}
