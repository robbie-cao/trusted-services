/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef REF_PARTITION_CONFIGURATOR_H
#define REF_PARTITION_CONFIGURATOR_H

#include <stdbool.h>
#include "service/block_storage/block_store/partitioned/partitioned_block_store.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * To support test, a reference storage partition configuration is used with
 * a set of different sized partitions. The total backend block store size
 * is kept as small as possible to allow the reference configuration to be
 * used with a ram backed store in environments where available memory is
 * constrained.
 */

#define REF_PARTITION_BACK_STORE_SIZE   (200)
#define REF_PARTITION_BLOCK_SIZE        (512)

/* About the right size for PSA storage */
#define REF_PARTITION_1_GUID            "92f7d53b-127e-432b-815c-9a95b80d69b7"
#define REF_PARTITION_1_STARTING_LBA    (0)
#define REF_PARTITION_1_ENDING_LBA      (95)

/* Also about the right size for PSA storage */
#define REF_PARTITION_2_GUID            "701456da-9b50-49b2-9722-47510f851ccd"
#define REF_PARTITION_2_STARTING_LBA    (96)
#define REF_PARTITION_2_ENDING_LBA      (191)

#define REF_PARTITION_3_GUID            "c39ef8a6-ec97-4883-aa64-025f40f7d922"
#define REF_PARTITION_3_STARTING_LBA    (192)
#define REF_PARTITION_3_ENDING_LBA      (195)

#define REF_PARTITION_4_GUID            "c3d82065-58f3-4fcb-a8fc-772434bfc91d"
#define REF_PARTITION_4_STARTING_LBA    (196)
#define REF_PARTITION_4_ENDING_LBA      (199)

/**
 * \brief Configures a partitioned_block_store with the reference configuration
 *
 * \param[in]  subject  The subject partitioned_block_store
 */
bool ref_partition_configure(struct partitioned_block_store *subject);

#ifdef __cplusplus
}
#endif

#endif /* REF_PARTITION_CONFIGURATOR_H */
