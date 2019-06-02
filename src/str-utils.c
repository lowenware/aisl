/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file str-utils.c
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief String utilities source file
 *
 * @see https://lowenware.com/aisl/
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "str-utils.h"


char *
str_copy(const char *source)
{
	int l = strlen(source);

	char *result = malloc(l+1);

	if (result)
		strcpy(result, source);

	return result;
}


int
str_cmpi(const char *s1, const char *s2)
{
	char c1, c2, r = 0;

	do {
		c1 = tolower(*(s1++));
		c2 = tolower(*(s2++));

		if ((r = c1-c2) != 0)
			break;

	} while (c1 != 0);

	return r;
}

