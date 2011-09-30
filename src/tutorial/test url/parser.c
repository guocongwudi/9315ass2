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
	if (purl->path == NULL) {
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
	int i;

	for (i = 0; i < strlen(purl->params); i++) {
		(purl->params)[i] = tolower((purl->params)[i]);
	}

	for (i = 0; i < strlen(purl->host); i++) {
		(purl->host)[i] = tolower((purl->host)[i]);
	}

	for (i = 0; i < strlen(purl->path); i++) {
		(purl->path)[i] = tolower((purl->path)[i]);
	}

	for (i = 0; i < strlen(purl->port); i++) {
		(purl->port)[i] = tolower((purl->port)[i]);
	}

	for (i = 0; i < strlen(purl->scheme); i++) {
		(purl->scheme)[i] = tolower((purl->scheme)[i]);
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

void main(void) {

	Url *url;
	Url *a;
	Url *b;
	char* line;
	char* line1;
	char* line2;

	line = "http://www.AAAA.com/song/play?ids=/song/playlist/id/7335983/type/3";
	line1 =
			"https://www.AAAA.com/song/play:80?ids=/song/playlist/id/7335983/type/3";
	line2 =
			"http://www.AAAA.com/song/play:80?ids=/song/playlist/id/7335983/type/4";
	url = parseURL(line);
	a = parseURL(line1);
	b = parseURL(line2);
	if (url == NULL
	)
		printf("this url is invalid");
	char *result = "";

	int url_len = 0;
	url_len = strlen(url->host) + strlen(url->params) + strlen(url->path)
			+ strlen(url->port) + strlen(url->scheme);

	result = (char *) malloc(url_len + 5);
	result = strcat(result, url->scheme);
	result = strcat(result, "://");

	result = strcat(result, url->host);
	result = strcat(result, ":");
	result = strcat(result, url->port);
	result = strcat(result, "/");

	if (url->path != NULL)
	{
		result = strcat(result, url->path);
	}
	if (url->params != NULL)
	{

		result = strcat(result, url->params);
	}

	printf("len = %d  ,%s", url_len, result);

//test the equal
	int isEqual = 1; // 1 means equal
	if (strcmp(a->host, b->host) == 0 && strcmp(a->path, b->path) == 0 &&

	strcmp(a->params, b->params) == 0 && isEqual)
		isEqual = 1;
	else
		isEqual = 0;

	if (isEqual) {
		if ((strcmp(a->scheme, b->scheme) == 0 && strcmp(a->port, b->port) == 0)
				|| ((strcmp(a->scheme, "http") == 0
						|| strcmp(b->scheme, "https") == 0)
						&& (strcmp(b->scheme, "http") == 0
								|| strcmp(a->scheme, "https") == 0)
						&& strcmp(a->port, b->port) == 0))
			isEqual = 1;
	} else
		isEqual = 0;

	printf("\n%d\n", isEqual);
	printf("\n================\n");

	char *str1 = "Hello Big Boy";
	char *t1;


	for ( t1 = strtok(str1," ");
	      t1 != NULL;
	      t1 = strtok(NULL, " ") )

	printf("%s\n",t1);


//	char *str1 = "Hello Big Boy";
//	char *t1;
//	printf("%s\n", str1);
//
//	for (t1 = strtok(str1, " "); t1 != NULL; t1 = strtok(NULL, " "))
//	{
//		printf("%s\n", t1);
//		int i;
//
//		for (i = 0; i < strlen(t1); i++) {
//			t1[i] = tolower(t1[i]);
//		}
//		printf("%s\n", t1);
//		printf("==\n");
//	}
//
//	printf("%s\n", str1);

}
