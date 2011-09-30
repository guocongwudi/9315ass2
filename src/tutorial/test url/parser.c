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
	int i=0;

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
	} else {
		if (strcmp(purl->scheme, "http") == 0)
			purl->port = "80";

		else
			purl->port = "403";

	}

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
void printParsedURL(Url *purl) {
	if (purl->scheme != NULL
	)
		printf("Scheme:  %s\n", purl->scheme);
	if (purl->host != NULL
	)
		printf("Host:    %s\n", purl->host);
	if (purl->port != NULL
	)
		printf("Port:    %s\n", purl->port);
	if (purl->path != NULL
	)
		printf("Path:    %s\n", purl->path);
	if (purl->params != NULL
	)
		printf("Params:  %s\n", purl->params);
}

void main(void) {

	Url *url;
	char* line;






	line = "http://www.AAAA.com/song/play?ids=/song/playlist/id/7335983/type/3";
	url = parseURL(line);
	if (url == NULL
	)
		printf("this url is invalid");
	else
		printParsedURL(url);
	char	   *result="";


		int url_len=0;
		url_len = strlen(url->host) +strlen(url->params) +strlen(url->path)+strlen(url->port)+strlen(url->scheme);


		result = (char *) malloc(url_len+5);
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

			result = strcat( result, url->params  );
		}


	printf("len = %d  ,%s",url_len,result);

}
