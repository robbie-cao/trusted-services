/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPC_DEMUX_H
#define RPC_DEMUX_H

#include <rpc/common/endpoint/rpc_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The default maximum number of output interfaces.  May be
 * overridden to meet needs of deployment if necessary.
 */
#ifndef RPC_DEMUX_MAX_OUTPUTS
#define RPC_DEMUX_MAX_OUTPUTS				(8)
#endif

/** \brief RPC demux
 *
 * An rpc_demux is an rpc_interface that demultiplexes incoming call requests
 * to 1..* output interfaces.  Use an rpc_demux when multiple service
 * providers are co-located and associated with a single RPC endpoint.
 */
struct rpc_demux
{
	struct rpc_interface input;
	struct rpc_interface *outputs[RPC_DEMUX_MAX_OUTPUTS];
};

/**
 * \brief Initialize an rpc_demux
 *
 * After initialization, the required number of output interfaces need
 * to be attached,
 *
 * \param[in] context   The instance to initialize
 *
 * \return The input rpc_interface
 */
struct rpc_interface *rpc_demux_init(struct rpc_demux *context);

/**
 * \brief Cleans up when the instance is no longer needed
 *
 * \param[in] context   The instance to de-initialize
 */
void rpc_demux_deinit(struct rpc_demux *context);

/**
 * \brief Attach an output interface
 *
 * \param[in] context   The rpc_demux instance
 * \param[in] iface_id	The interface id (small integer)
 * \param[in] output	The interface to attach
 */
void rpc_demux_attach(struct rpc_demux *context,
	unsigned int iface_id, struct rpc_interface *output);

/**
 * \brief Dettach an output interface
 *
 * \param[in] context   The rpc_demux instance
 * \param[in] iface_id	The interface id (small integer)
 */
void rpc_demux_dettach(struct rpc_demux *context,
	unsigned int iface_id);

#ifdef __cplusplus
}
#endif

#endif /* RPC_DEMUX_H */
