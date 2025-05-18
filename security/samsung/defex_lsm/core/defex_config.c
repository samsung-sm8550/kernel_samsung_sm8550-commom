// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */

#include "include/defex_config.h"
#include "include/defex_internal.h"

#if defined(DEFEX_KUNIT_ENABLED) || defined(DEFEX_DEBUG_ENABLE)
__visible_for_testing unsigned int global_features_status;
#else
static unsigned int global_features_status __ro_after_init;
#endif

void defex_init_features(void)
{
#ifdef DEFEX_PED_ENABLE
global_features_status |= FEATURE_CHECK_CREDS;
#endif /* DEFEX_PED_ENABLE */
#ifdef DEFEX_INTEGRITY_ENABLE
global_features_status |= FEATURE_INTEGRITY;
#ifdef DEFEX_PERMISSIVE_INT
global_features_status |= FEATURE_INTEGRITY_SOFT;
#endif /* DEFEX_PERMISSIVE_INT */
#endif /* DEFEX_INTEGRITY_ENABLE */
#ifdef DEFEX_SAFEPLACE_ENABLE
global_features_status |= FEATURE_SAFEPLACE;
#ifdef DEFEX_PERMISSIVE_SP
global_features_status |= FEATURE_SAFEPLACE_SOFT;
#endif /* DEFEX_PERMISSIVE_SP */
#endif /* DEFEX_SAFEPLACE_ENABLE */
#ifdef DEFEX_IMMUTABLE_ENABLE
global_features_status |= FEATURE_IMMUTABLE;
#ifdef DEFEX_PERMISSIVE_IM
global_features_status |= FEATURE_IMMUTABLE_SOFT;
#endif /* DEFEX_PERMISSIVE_IM */
#endif /* DEFEX_IMMUTABLE_ENABLE */
#ifdef DEFEX_IMMUTABLE_ROOT_ENABLE
global_features_status |= FEATURE_IMMUTABLE_ROOT;
#ifdef DEFEX_PERMISSIVE_IMR
global_features_status |= FEATURE_IMMUTABLE_ROOT_SOFT;
#endif /* DEFEX_PERMISSIVE_IMR */
#endif /* DEFEX_IMMUTABLE_ROOT_ENABLE */
#ifdef DEFEX_TRUSTED_MAP_ENABLE
global_features_status |= (DEFEX_TM_DEBUG_VIOLATIONS | FEATURE_TRUSTED_MAP);
#ifdef DEFEX_PERMISSIVE_TM
global_features_status |= FEATURE_TRUSTED_MAP_SOFT;
#endif /* DEFEX_PERMISSIVE_TM */
#endif /* DEFEX_TRUSTED_MAP_ENABLE */
}

#if defined(DEFEX_DEBUG_ENABLE) || defined(DEFEX_LOG_BUFFER_ENABLE) \
	|| defined(DEFEX_LOG_FILE_ENABLE)
void defex_set_features(unsigned int set_features)
{
	global_features_status = set_features;
}
#endif /* DEFEX_DEBUG_ENABLE || DEFEX_LOG_BUFFER_ENABLE || DEFEX_LOG_FILE_ENABLE */

unsigned int defex_get_features(void)
{
	return global_features_status;
}
