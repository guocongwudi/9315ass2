/*
 * $PostgreSQL: pgsql/src/tutorial/complex.c,v 1.15 2009/06/11 14:49:15 momjian Exp $
 *
 ******************************************************************************
 This file contains routines that can be bound to a Postgres backend and
 called by the backend in the process of processing queries.  The calling
 format for these routines is dictated by Postgres architecture.
 ******************************************************************************/
#include "postgres.h"
#include "fmgr.h"
#include "libpq/pqformat.h"		/* needed for send/recv functions */
PG_MODULE_MAGIC
;

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
static int isLessthan(Url *, Url *);
static int isGreatthan(Url *, Url *);
static int isUrlEqual(Url* , Url* ) ;

int url_abs_cmp_internal(Url *, Url*);
int url_abs_cmp_internal(Url * a, Url* b) {

}

char *str_n_dup(char *str, int n) {
	char *new = palloc(n + 12);
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
	int i;
	for (i = 0; i < strlen(purl->scheme); i++) {
		purl->scheme[i] = tolower(purl->scheme[i]);
	}

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
			purl->port = palloc(4);
			strcpy(purl->port, "80");
			purl->port[2] = '\0';
		} else {
			purl->port = palloc(4);
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

			while (*d != '\0' && *d != '?') {
				d++;
			}
			purl->path = str_n_dup(c, d - c);
			char *tmp = palloc(strlen(purl->path)+1);
			int douslash = 0;
			int index = 0;
			for (i = 0; i < strlen(purl->path); i++) {
				if (purl->path[i] == '/') {
					if (douslash != 1) {
						tmp[index++] = purl->path[i];
					}
					douslash = 1;
				}

				else {
					douslash = 0;
					tmp[index++] = purl->path[i];
				}

			}
			purl->path = str_n_dup(tmp, index);
		}
	}
//	********************merge**********************//
	//	default path
	if (purl->path == NULL) {
		purl->path = palloc(11);
		strcpy(purl->path, "index.html");
		purl->path[10] = '\0';
	}
//		****************merge**********************//
	else
		{
	        int path_len= strlen(purl->path );
	        char * p=purl->path;
	        while(*p!='\0'&& *p!='?')
	        	p++;
	        if(*(p-1)=='/'){

	          strcat(purl->path,"index.html");
	        }




	// copy params, if any
	if (*d != '\0') {
		c = d = d + 1; // skip over '?'
		if (*d != '\0') {
			purl->params = strdup(c);
		}
	}

	for (i = 0; purl->params != NULL && i < strlen(purl->params); i++) {
//		printf("%s \n", purl->params[i]);
		purl->params[i] = tolower(purl->params[i]);
	}

	for (i = 0; i < strlen(purl->host); i++) {
		purl->host[i] = tolower(purl->host[i]);
	}

	for (i = 0; i < strlen(purl->path); i++) {
		purl->path[i] = tolower(purl->path[i]);
	}

	for (i = 0; i < strlen(purl->port); i++) {
		purl->port[i] = tolower(purl->port[i]);
	}

	return purl;
}

Url *makeParsedURL() {
	Url *purl;
	if ((purl = palloc(sizeof(Url))) == NULL
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
Datum url_in(PG_FUNCTION_ARGS);
Datum url_out(PG_FUNCTION_ARGS);
Datum url_recv(PG_FUNCTION_ARGS);
Datum url_send(PG_FUNCTION_ARGS);
//Datum url_add(PG_FUNCTION_ARGS);
Datum url_abs_lt(PG_FUNCTION_ARGS);
Datum url_abs_le(PG_FUNCTION_ARGS);
Datum url_abs_eq(PG_FUNCTION_ARGS);
Datum url_abs_ge(PG_FUNCTION_ARGS);
Datum url_abs_gt(PG_FUNCTION_ARGS);
//Datum url_abs_cmp(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(url_in);

Datum url_in(PG_FUNCTION_ARGS) {

	char *str = PG_GETARG_CSTRING(0);
	char * result=NULL;
	text * vardata;


	fprintf(stderr, "come from out----------------##--%s\n",str);
	Url *url = parseURL(str);



	if (url != NULL) {
		int len = strlen(url->host)  + strlen(url->path)
				+ strlen(url->port) + strlen(url->scheme);
		 if(url->params!=NULL){
		            	len+=strlen(url->params);
		            }

		result = malloc(len+6);

		strcpy(result, url->scheme);
		strcat(result, "://");
		strcat(result, url->host);
		strcat(result, ":");
		strcat(result, url->port);
		strcat(result, "/");
		strcat(result, url->path);
		if (url->params != NULL) {
			strcat(result, "?");
			strcat(result, url->params);
		}

	}

	vardata = (text *) cstring_to_text(result);



	//char * tmp = text_to_cstring((text *) vardata);
//	fprintf(stderr, "the result: -----%s\n", tmp);

	PG_RETURN_POINTER(vardata);

}

PG_FUNCTION_INFO_V1(url_out);

Datum url_out(PG_FUNCTION_ARGS) {


	text * x = (text *) PG_GETARG_POINTER(0);

	char *str = text_to_cstring((text *) x);
	fprintf(stderr, "the result: -----%s\n", x);
	 PG_RETURN_CSTRING(str);
}

/*****************************************************************************
 * Binary Input/Output functions
 *
 * These are optional.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(url_recv);

Datum url_recv(PG_FUNCTION_ARGS) {
	fprintf(stderr, "come from rec------------------start\n");
	StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
	char *str;
text * result;

	str = pq_getstring(buf);
result=(text *) cstring_to_text(str);
	fprintf(stderr, "come from rev------------------end\n");
	PG_RETURN_POINTER(result);



}

PG_FUNCTION_INFO_V1(url_send);

Datum url_send(PG_FUNCTION_ARGS) {
	//fprintf(stderr, "come from @send------------------start\n");
	text *result = (text *) PG_GETARG_POINTER(0);
	//int a =strlen(result);
	//fprintf(stderr, "come from @send-------------%d-----start22\n",a);
	char *str = text_to_cstring((text *) result);
//	fprintf(stderr, "come from @send------------------start33\n");
///	fprintf(stderr, "come from send------------------%s =\n",str);

	StringInfoData buf;
	pq_begintypsend(&buf);
	pq_sendstring(&buf, result);

//	fprintf(stderr, "come from out------------------end\n");
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

Datum url_abs_lt(PG_FUNCTION_ARGS) {
	Url *a = (Url *) PG_GETARG_POINTER(0);
	Url *b = (Url *) PG_GETARG_POINTER(1);

	int isEqual = isUrlEqual(a, b);
	int isLessThan = !isEqual;
	if (!isEqual) {
		isLessThan = isLessthan(a, b);
	}
	//none zero return true
	PG_RETURN_BOOL(isLessThan);
}

PG_FUNCTION_INFO_V1(isLessthan);

int isLessthan(Url *a, Url *b) {
	int isLessThan = 0;
	char * aa = palloc(strlen(a->host) + strlen(a->path) + 1);
	char * bb = palloc(strlen(b->host) + strlen(b->path) + 1);
	strcpy(aa, a->host);
	strcpy(bb, b->host);
	strcat(aa, "/");
	strcat(bb, "/");
	strcat(aa, a->path);
	strcat(bb, b->path);
	int i = 0;
	for (i = strlen(aa) - 1; i >= 0; i--) {
		if (aa[i] != '/')
			continue;
		else {
			aa[i] = '\0';
			break;
		}
	}
	for (i = strlen(bb) - 1; i >= 0; i--) {
		if (bb[i] != '/')
			continue;
		else {
			bb[i] = '\0';
			break;
		}
	}
	//if aa is less than bb, then aa.lengh < bb.lenth
	if (strlen(aa) <= strlen(bb))
		isLessThan = 0;
	else {
		//			strstr() Return Value
		//			A pointer to the first occurrence in str1 of any of the entire sequence of characters specified in str2, or a null pointer if the sequence is not present in str1.
		if (strstr(aa, bb) == NULL
			)
			isLessThan = 0;
		else
			isLessThan = 1;
	}
	return isLessThan;
}
PG_FUNCTION_INFO_V1(url_abs_le);

Datum url_abs_le(PG_FUNCTION_ARGS) {
	Url *a = (Url *) PG_GETARG_POINTER(0);
	Url *b = (Url *) PG_GETARG_POINTER(1);
	int isEqual = isUrlEqual(a, b);
	int isLessThan = isLessthan(a, b);
	int isEquOrLess = 0;
	if (isEqual || isLessThan)
		isEquOrLess = 1;
	else
		isEquOrLess = 0;

	PG_RETURN_BOOL(isEquOrLess);
}


static int isUrlEqual(Url* a, Url* b) {
	int isEqual = 1; // 1 means equal
	if (strcmp(a->host, b->host) == 0 && strcmp(a->path, b->path) == 0 &&

	strcmp(a->params, b->params) == 0 && isEqual) {
		isEqual = 1;
	} else
		isEqual = 0;

	if (isEqual) {
		if ((strcmp(a->scheme, b->scheme) == 0)
				&& (strcmp(a->port, b->port) == 0)) {
			isEqual = 1;
		} else if ((strcmp(a->scheme, b->scheme) != 0)
				&& (strcmp(a->port, b->port) == 0)) {
			isEqual = 1;
		} else {
			isEqual = 0;
		}
	}
	return isEqual;
}

PG_FUNCTION_INFO_V1(url_abs_eq);

Datum url_abs_eq(PG_FUNCTION_ARGS) {
printf("ddddddddddddddddddddddddddd");
	text * x = (text *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	char *str = text_to_cstring((text *) x);
	text * x1 = (text *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
	char *str1 = text_to_cstring((text *) x1);

	Url *a = parseURL(str);
	printf("url a is %s", a);
	Url *b =  parseURL(str1);
	printf("url b is %s", b);

	int isEqual = isUrlEqual(a, b);

	PG_RETURN_BOOL(isEqual);
}

PG_FUNCTION_INFO_V1(url_abs_ge);

Datum url_abs_ge(PG_FUNCTION_ARGS) {
	Url *a = (Url *) PG_GETARG_POINTER(0);
	Url *b = (Url *) PG_GETARG_POINTER(1);
	int isEqual = isUrlEqual(a, b);
	int isGreatThan = isGreatthan(a, b);
	int isEquOrGreat = 0;
	if (isEqual || isGreatThan)
		isEquOrGreat = 1;
	else
		isEquOrGreat = 0;

	PG_RETURN_BOOL(isEquOrGreat);
}

PG_FUNCTION_INFO_V1(url_abs_gt);

Datum url_abs_gt(PG_FUNCTION_ARGS) {
	Url *a = (Url *) PG_GETARG_POINTER(0);
	Url *b = (Url *) PG_GETARG_POINTER(1);
	int isEqual = isUrlEqual(a, b);
	int isgreatThan = !isEqual;
	if (!isEqual) {
		isgreatThan = isGreatthan(a, b);

	}
	//none zero return true
	PG_RETURN_BOOL(isgreatThan);

}



static int isGreatthan(Url *a, Url *b) {
	int isGreatThan = 0;
	char * aa = malloc(strlen(a->host) + strlen(a->path) + 1);
	char * bb = malloc(strlen(b->host) + strlen(b->path) + 1);
	strcpy(aa, a->host);
	strcpy(bb, b->host);
	int isLessthan(Url *a, Url *b) {
		int isLessThan = 0;
		char * aa = malloc(strlen(a->host) + strlen(a->path) + 1);
		char * bb = malloc(strlen(b->host) + strlen(b->path) + 1);
		strcpy(aa, a->host);
		strcpy(bb, b->host);
		strcat(aa, "/");
		strcat(bb, "/");
		strcat(aa, a->path);
		strcat(bb, b->path);
		int i = 0;
		for (i = strlen(aa) - 1; i >= 0; i--) {
			if (aa[i] != '/')
				continue;
			else {
				aa[i] = '\0';
				break;
			}
		}
		for (i = strlen(bb) - 1; i >= 0; i--) {
			if (bb[i] != '/')
				continue;
			else {
				bb[i] = '\0';
				break;
			}
		}
		//if aa is less than bb, then aa.lengh < bb.lenth
		if (strlen(aa) <= strlen(bb))
			isLessThan = 0;
		else {
			//			strstr() Return Value
			//			A pointer to the first occurrence in str1 of any of the entire sequence of characters specified in str2, or a null pointer if the sequence is not present in str1.
			if (strstr(aa, bb) == NULL
				)
				isLessThan = 0;
			else
				isLessThan = 1;
		}
		return isLessThan;
	}
	strcat(aa, "/");
	strcat(bb, "/");
	strcat(aa, a->path);
	strcat(bb, b->path);
	int i = 0;
	for (i = strlen(aa) - 1; i >= 0; i--) {
		if (aa[i] != '/')
			continue;
		else {
			aa[i] = '\0';
			break;
		}
	}
	for (i = strlen(bb) - 1; i >= 0; i--) {
		if (bb[i] != '/')
			continue;
		else {
			bb[i] = '\0';
			break;
		}
	}
	//if aa is less than bb, then aa.lengh < bb.lenth
	if (strlen(aa) >= strlen(bb))
		isGreatThan = 0;
	else {
		//			strstr() Return Value
		//			A pointer to the first occurrence in str1 of any of the entire sequence of characters specified in str2, or a null pointer if the sequence is not present in str1.
		if (strstr(aa, bb) == NULL
			)
			isGreatThan = 1;
		else
			isGreatThan = 0;
	}
	return isGreatThan;
}

//PG_FUNCTION_INFO_V1(url_abs_cmp);
//
//Datum url_abs_cmp(PG_FUNCTION_ARGS) {
//	Url *a = (Url *) PG_GETARG_POINTER(0);
//	Url *b = (Url *) PG_GETARG_POINTER(1);
//
//	PG_RETURN_INT32(url_abs_cmp_internal(a, b));
//}
