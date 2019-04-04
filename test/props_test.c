/*
 * Copyright (c) 2015 Hanspeter Portner (dev@open-music-kontrollers.ch)
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the Artistic License 2.0 as published by
 * The Perl Foundation.
 *
 * This source is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Artistic License 2.0 for more details.
 *
 * You should have received a copy of the Artistic License 2.0
 * along the source as a COPYING file. If not, obtain it from
 * http://www.perlfoundation.org/artistic_license_2_0.
 */


#include <assert.h>

#include <props.h>

#define MAX_URIDS 512

typedef struct _urid_t urid_t;
typedef void (*test_t)(props_t *props);

struct _urid_t {
	LV2_URID urid;
	char *uri;
};

static urid_t urids [MAX_URIDS];
static LV2_URID urid;

static LV2_URID
_map(LV2_URID_Map_Handle instance __attribute__((unused)), const char *uri)
{
	urid_t *itm;
	for(itm=urids; itm->urid; itm++)
	{
		if(!strcmp(itm->uri, uri))
			return itm->urid;
	}

	assert(urid + 1 < MAX_URIDS);

	// create new
	itm->urid = ++urid;
	itm->uri = strdup(uri);

	return itm->urid;
}

static LV2_URID_Map map = {
	.handle = NULL,
	.map = _map
};

static void
_test_1(props_t *props)
{
	assert(props);
	//FIXME
}

static void
_test_2(props_t *props)
{
	assert(props);
	//FIXME
}

static const test_t tests [] = {
	_test_1,
	_test_2,
	NULL
};

#define PROPS_PREFIX		"http://open-music-kontrollers.ch/lv2/props#"
#define PROPS_TEST_URI	PROPS_PREFIX"test"

#define MAX_NPROPS 1

typedef struct _plugstate_t plugstate_t;

struct _plugstate_t {
	uint32_t dummy;
};

static const props_def_t defs [MAX_NPROPS] = {
	{
		.property = PROPS_PREFIX"dummy",
		.offset = offsetof(plugstate_t, dummy),
		.type = LV2_ATOM__Int
	}
};

int
main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	static props_t props;
	static plugstate_t state;
	static plugstate_t stash;

	for(const test_t *test = tests; *test; test++)
	{
		memset(&props, 0, sizeof(props));
		memset(&state, 0, sizeof(state));
		memset(&stash, 0, sizeof(stash));

		assert(props_init(&props, PROPS_PREFIX"subj", defs, MAX_NPROPS,
			&state, &stash, &map, NULL) == 1);

		(*test)(&props);
	}

	for(unsigned i=0; i<urid; i++)
	{
		urid_t *itm = &urids[i];

		free(itm->uri);
	}

	return 0;
}
