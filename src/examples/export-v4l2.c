/* PipeWire
 * Copyright (C) 2017 Wim Taymans <wim.taymans@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <sys/mman.h>

#include <SDL2/SDL.h>

#include <spa/type-map.h>
#include <spa/format-utils.h>
#include <spa/video/format-utils.h>
#include <spa/format-builder.h>
#include <spa/props.h>
#include <spa/lib/debug.h>

#include <pipewire/pipewire.h>
#include <pipewire/sig.h>
#include <pipewire/module.h>
#include <pipewire/node-factory.h>

struct type {
	uint32_t format;
	uint32_t props;
	struct spa_type_meta meta;
	struct spa_type_data data;
	struct spa_type_media_type media_type;
	struct spa_type_media_subtype media_subtype;
	struct spa_type_format_video format_video;
	struct spa_type_video_format video_format;
};

static inline void init_type(struct type *type, struct spa_type_map *map)
{
	type->format = spa_type_map_get_id(map, SPA_TYPE__Format);
	type->props = spa_type_map_get_id(map, SPA_TYPE__Props);
	spa_type_meta_map(map, &type->meta);
	spa_type_data_map(map, &type->data);
	spa_type_media_type_map(map, &type->media_type);
	spa_type_media_subtype_map(map, &type->media_subtype);
	spa_type_format_video_map(map, &type->format_video);
	spa_type_video_format_map(map, &type->video_format);
}

struct data {
	struct type type;

	bool running;
	struct pw_loop *loop;

	struct pw_core *core;

	struct pw_remote *remote;
	struct pw_listener on_state_changed;

	struct pw_node *node;
};

static void make_node(struct data *data)
{
	struct pw_node_factory *factory;
	struct pw_properties *props;

        factory = pw_core_find_node_factory(data->core, "spa-node-factory");
        props = pw_properties_new("spa.library.name", "v4l2/libspa-v4l2",
                                  "spa.factory.name", "v4l2-source", NULL);
        data->node = pw_node_factory_create_node(factory, NULL, "v4l2-source", props);

	pw_node_register(data->node);

	pw_remote_export(data->remote, data->node);
}

static void on_state_changed(struct pw_listener *listener, struct pw_remote *remote)
{
	struct data *data = SPA_CONTAINER_OF(listener, struct data, on_state_changed);

	switch (remote->state) {
	case PW_REMOTE_STATE_ERROR:
		printf("remote error: %s\n", remote->error);
		data->running = false;
		break;

	case PW_REMOTE_STATE_CONNECTED:
		make_node(data);
		break;

	default:
		printf("remote state: \"%s\"\n", pw_remote_state_as_string(remote->state));
		break;
	}
}


int main(int argc, char *argv[])
{
	struct data data = { 0, };

	pw_init(&argc, &argv);

	data.loop = pw_loop_new();
	data.running = true;
	data.core = pw_core_new(data.loop, NULL);
        data.remote = pw_remote_new(data.core, NULL);

	pw_module_load(data.core, "libpipewire-module-spa-node-factory", NULL);

	init_type(&data.type, data.core->type.map);

	spa_debug_set_type_map(data.core->type.map);

	pw_signal_add(&data.remote->state_changed, &data.on_state_changed, on_state_changed);

        pw_remote_connect(data.remote);

	pw_loop_enter(data.loop);
	while (data.running) {
		pw_loop_iterate(data.loop, -1);
	}
	pw_loop_leave(data.loop);

	pw_loop_destroy(data.loop);

	return 0;
}
