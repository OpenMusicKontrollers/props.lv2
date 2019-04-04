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

#define PROPS_PREFIX		"http://open-music-kontrollers.ch/lv2/props#"
#define PROPS_TEST_URI	PROPS_PREFIX"test"

#define MAX_NPROPS 5

typedef struct _plugstate_t plugstate_t;
typedef struct _urid_t urid_t;
typedef void (*test_t)(props_t *props);

struct _plugstate_t {
	int32_t b32;
	int32_t i32;
	int64_t i64;
	float f32;
	double f64;
};

struct _urid_t {
	LV2_URID urid;
	char *uri;
};

static urid_t urids [MAX_URIDS];
static LV2_URID nurid;

static void
_map_deinit()
{
	for(urid_t *itm=urids; itm->urid; itm++)
	{
		free(itm->uri);
	}
}

static LV2_URID
_map(LV2_URID_Map_Handle instance __attribute__((unused)), const char *uri)
{
	urid_t *itm;
	for(itm=urids; itm->urid; itm++)
	{
		if(!strcmp(itm->uri, uri))
		{
			return itm->urid;
		}
	}

	assert(nurid + 1 < MAX_URIDS);

	// create new
	itm->urid = ++nurid;
	itm->uri = strdup(uri);
	assert(itm->uri);

	return itm->urid;
}

static LV2_URID_Map map = {
	.handle = NULL,
	.map = _map
};

static const props_def_t defs [MAX_NPROPS] = {
	{
		.property = PROPS_PREFIX"b32",
		.offset = offsetof(plugstate_t, b32),
		.type = LV2_ATOM__Bool
	},
	{
		.property = PROPS_PREFIX"i32",
		.offset = offsetof(plugstate_t, i32),
		.type = LV2_ATOM__Int
	},
	{
		.property = PROPS_PREFIX"i64",
		.offset = offsetof(plugstate_t, i64),
		.type = LV2_ATOM__Long
	},
	{
		.property = PROPS_PREFIX"f32",
		.offset = offsetof(plugstate_t, f32),
		.type = LV2_ATOM__Float
	},
	{
		.property = PROPS_PREFIX"f64",
		.offset = offsetof(plugstate_t, f64),
		.type = LV2_ATOM__Double
	}
};

static plugstate_t state;
static plugstate_t stash;

static void
_test_1(props_t *props)
{
	assert(props);

	for(unsigned i = 0; i < MAX_NPROPS; i++)
	{
		const props_def_t *def = &defs[i];

		const LV2_URID property = props_map(props, def->property);
		assert(property != 0);

		props_impl_t *impl = _props_impl_get(props, property);
		assert(impl);

		const LV2_URID type = map.map(map.handle, def->type);
		const LV2_URID access = map.map(map.handle, def->access
			? def->access : LV2_PATCH__writable);

		assert(impl->property == property);
		assert(impl->type == type);
		assert(impl->access == access);

		assert(impl->def == def);

		assert(atomic_load(&impl->state) == PROP_STATE_NONE);
		assert(impl->stashing == false);

		switch(i)
		{
			case 0:
			{
				assert(impl->value.size == sizeof(state.b32));
				assert(impl->value.body == &state.b32);

				assert(impl->stash.size == sizeof(stash.b32));
				assert(impl->stash.body == &stash.b32);
			} break;
			case 1:
			{
				assert(impl->value.size == sizeof(state.i32));
				assert(impl->value.body == &state.i32);

				assert(impl->stash.size == sizeof(stash.i32));
				assert(impl->stash.body == &stash.i32);
			} break;
			case 2:
			{
				assert(impl->value.size == sizeof(state.i64));
				assert(impl->value.body == &state.i64);

				assert(impl->stash.size == sizeof(stash.i64));
				assert(impl->stash.body == &stash.i64);
			} break;
			case 3:
			{
				assert(impl->value.size == sizeof(state.f32));
				assert(impl->value.body == &state.f32);

				assert(impl->stash.size == sizeof(stash.f32));
				assert(impl->stash.body == &stash.f32);
			} break;
			case 4:
			{
				assert(impl->value.size == sizeof(state.f64));
				assert(impl->value.body == &state.f64);

				assert(impl->stash.size == sizeof(stash.f64));
				assert(impl->stash.body == &stash.f64);
			} break;
			default:
			{
				assert(false);
			} break;
		}
	}
}

static const test_t tests [] = {
	_test_1,
	NULL
};

int
main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	static props_t props;

	for(const test_t *test = tests; *test; test++)
	{
		memset(&props, 0, sizeof(props));
		memset(&state, 0, sizeof(state));
		memset(&stash, 0, sizeof(stash));

		assert(props_init(&props, PROPS_PREFIX"subj", defs, MAX_NPROPS,
			&state, &stash, &map, NULL) == 1);

		(*test)(&props);
	}

	_map_deinit();

	return 0;
}
