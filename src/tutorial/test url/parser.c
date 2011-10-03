#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

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
int isLessthan(Url *, Url *);
int isGreatthan(Url *, Url *);

int url_abs_cmp_internal(Url *, Url*);
int url_abs_cmp_internal(Url * a, Url* b) {

}

char *str_n_dup(char *str, int n) {
	char *new = palloc(n + 1);
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
			purl->port = palloc(40);
			strcpy(purl->port, "80");
			purl->port[2] = '\0';
		} else {
			purl->port = palloc(40);
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
			char *tmp = palloc(strlen(purl->path)+100);
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
		purl->path = palloc(110);
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
