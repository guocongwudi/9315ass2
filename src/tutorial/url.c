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


PG_MODULE_MAGIC;

typedef struct Url
{
	char * url;
	double		x;
	double		y;
}	Url;

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
{
	char	   *str = PG_GETARG_CSTRING(0);
	double		x,
				y;
	Url    *result;

	if (sscanf(str, " ( %lf , %lf )", &x, &y) != 2)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for Url: \"%s\"",
						str)));

	result = (Url *) palloc(sizeof(Url));
	result->x = x;
	result->y = y;
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(url_out);

Datum
url_out(PG_FUNCTION_ARGS)
{
	Url    *complex = (Url *) PG_GETARG_POINTER(0);
	char	   *result;

	result = (char *) palloc(100);
	snprintf(result, 100, "(%g,%g)", complex->x, complex->y);
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
	result->x = pq_getmsgfloat8(buf);
	result->y = pq_getmsgfloat8(buf);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(url_send);

Datum
url_send(PG_FUNCTION_ARGS)
{
	Url    *complex = (Url *) PG_GETARG_POINTER(0);
	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendfloat8(&buf, complex->x);
	pq_sendfloat8(&buf, complex->y);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * New Operators
 *
 * A practical Url datatype would provide much more than this, of course.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(url_add);

Datum
url_add(PG_FUNCTION_ARGS)
{
	Url    *a = (Url *) PG_GETARG_POINTER(0);
	Url    *b = (Url *) PG_GETARG_POINTER(1);
	Url    *result;

	result = (Url *) palloc(sizeof(Url));
	result->x = a->x + b->x;
	result->y = a->y + b->y;
	PG_RETURN_POINTER(result);
}


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

#define Mag(c)	((c)->x*(c)->x + (c)->y*(c)->y)

static int
url_abs_cmp_internal(Url * a, Url * b)
{
	double		amag = Mag(a),
				bmag = Mag(b);

	if (amag < bmag)
		return -1;
	if (amag > bmag)
		return 1;
	return 0;
}


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
