/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */

#ifndef __CONFIG_SECURITY_DEFEX_CONFIG_H
#define __CONFIG_SECURITY_DEFEX_CONFIG_H

#include <linux/version.h>

#define KERNEL_VER_LESS(x, y, z)	(KERNEL_VERSION(x, y, z) > LINUX_VERSION_CODE)
#define KERNEL_VER_LTE(x, y, z)		(KERNEL_VERSION(x, y, z) >= LINUX_VERSION_CODE)
#define KERNEL_VER_GTE(x, y, z)		(KERNEL_VERSION(x, y, z) <= LINUX_VERSION_CODE)

/* Uncomment for Kernels, that require it */
#define STRICT_UID_TYPE_CHECKS			1

#ifndef __ASSEMBLY__
void defex_init_features(void);
void defex_set_features(unsigned int set_features);
unsigned int defex_get_features(void);
#endif /* __ASSEMBLY__ */

#endif /* CONFIG_SECURITY_DEFEX_CONFIG_H */
