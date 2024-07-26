/*
 * netprintf - Simple network print formatted.
 *
 * Author: Philip R. Simonson
 * Date:   07/26/2024
 *
 */

#define _DEFAULT_SOURCE 

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "prs/network.h"
#include "netprintf.h"

/* Print formatted string to socket.
 */
long int socket_printf(SOCKET s, const char *msg, ...)
{
	long int rc;
	char *data;
	va_list ap;

	rc = snprintf(NULL, 0, msg, ap);
	data = (char*)malloc(sizeof(char)*(rc+1));
	if(data == NULL) {
		return -1;
	}

	va_start(ap, msg);
	(void)vsprintf(data, msg, ap);
	va_end(ap);

	rc = send(s, data, strlen(data), 0);
	free(data);

	if(rc < 0) {
		return -1;
	}

	return rc;
}
