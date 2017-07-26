/* PipeWire
 * Copyright (C) 2016 Wim Taymans <wim.taymans@gmail.com>
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

#ifndef __PIPEWIRE_TRANSPORT_H__
#define __PIPEWIRE_TRANSPORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include <spa/defs.h>
#include <spa/node.h>

#include <pipewire/mem.h>
#include <pipewire/sig.h>

/** information about the transport region \memberof pw_transport */
struct pw_transport_info {
	int memfd;		/**< the memfd of the transport area */
	uint32_t offset;	/**< offset to map \a memfd at */
	uint32_t size;		/**< size of memfd mapping */
};

/** Shared structure between client and server \memberof pw_transport */
struct pw_transport_area {
	uint32_t max_input_ports;	/**< max input ports of the node */
	uint32_t n_input_ports;		/**< number of input ports of the node */
	uint32_t max_output_ports;	/**< max output ports of the node */
	uint32_t n_output_ports;	/**< number of output ports of the node */
};

/** \class pw_transport
 *
 * \brief Transport object
 *
 * The transport object contains shared data and ringbuffers to exchange
 * events and data between the server and the client in a low-latency and
 * lockfree way.
 */
struct pw_transport {
	/** Emited when the transport is destroyed */
	PW_SIGNAL(destroy_signal, (struct pw_listener *listener, struct pw_transport *trans));

	struct pw_transport_area *area;		/**< the transport area */
	struct spa_port_io *inputs;		/**< array of input port io */
	struct spa_port_io *outputs;		/**< array of output port io */
	void *input_data;			/**< input memory for ringbuffer */
	struct spa_ringbuffer *input_buffer;	/**< ringbuffer for input memory */
	void *output_data;			/**< output memory for ringbuffer */
	struct spa_ringbuffer *output_buffer;	/**< ringbuffer for output memory */

	void *user_data;
};

struct pw_transport *
pw_transport_new(uint32_t max_input_ports, uint32_t max_output_ports, size_t user_data_size);

struct pw_transport *
pw_transport_new_from_info(struct pw_transport_info *info, size_t user_data_size);

void
pw_transport_destroy(struct pw_transport *trans);

int
pw_transport_get_info(struct pw_transport *trans, struct pw_transport_info *info);

int
pw_transport_add_event(struct pw_transport *trans, struct spa_event *event);

int
pw_transport_next_event(struct pw_transport *trans, struct spa_event *event);

int
pw_transport_parse_event(struct pw_transport *trans, void *event);

#define PW_TYPE_EVENT__Transport            SPA_TYPE_EVENT_BASE "Transport"
#define PW_TYPE_EVENT_TRANSPORT_BASE        PW_TYPE_EVENT__Transport ":"

#define PW_TYPE_EVENT_TRANSPORT__HaveOutput	PW_TYPE_EVENT_TRANSPORT_BASE "HaveOutput"
#define PW_TYPE_EVENT_TRANSPORT__NeedInput	PW_TYPE_EVENT_TRANSPORT_BASE "NeedInput"
#define PW_TYPE_EVENT_TRANSPORT__ReuseBuffer	PW_TYPE_EVENT_TRANSPORT_BASE "ReuseBuffer"
#define PW_TYPE_EVENT_TRANSPORT__ProcessInput	PW_TYPE_EVENT_TRANSPORT_BASE "ProcessInput"
#define PW_TYPE_EVENT_TRANSPORT__ProcessOutput	PW_TYPE_EVENT_TRANSPORT_BASE "ProcessOutput"

struct pw_type_event_transport {
	uint32_t HaveOutput;
	uint32_t NeedInput;
	uint32_t ReuseBuffer;
	uint32_t ProcessInput;
	uint32_t ProcessOutput;
};

static inline void
pw_type_event_transport_map(struct spa_type_map *map, struct pw_type_event_transport *type)
{
	if (type->HaveOutput == 0) {
		type->HaveOutput = spa_type_map_get_id(map, PW_TYPE_EVENT_TRANSPORT__HaveOutput);
		type->NeedInput = spa_type_map_get_id(map, PW_TYPE_EVENT_TRANSPORT__NeedInput);
		type->ReuseBuffer = spa_type_map_get_id(map, PW_TYPE_EVENT_TRANSPORT__ReuseBuffer);
		type->ProcessInput = spa_type_map_get_id(map, PW_TYPE_EVENT_TRANSPORT__ProcessInput);
		type->ProcessOutput = spa_type_map_get_id(map, PW_TYPE_EVENT_TRANSPORT__ProcessOutput);
	}
}

struct pw_event_transport_reuse_buffer_body {
	struct spa_pod_object_body body;
	struct spa_pod_int port_id;
	struct spa_pod_int buffer_id;
};

struct pw_event_transport_reuse_buffer {
	struct spa_pod pod;
	struct pw_event_transport_reuse_buffer_body body;
};

#define PW_EVENT_TRANSPORT_REUSE_BUFFER_INIT(type,port_id,buffer_id)		\
	SPA_EVENT_INIT_COMPLEX(struct pw_event_transport_reuse_buffer,		\
		sizeof(struct pw_event_transport_reuse_buffer_body), type,	\
		SPA_POD_INT_INIT(port_id),					\
		SPA_POD_INT_INIT(buffer_id))


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __PIPEWIRE_TRANSPORT_H__ */
