// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */

#include <kunit/test.h>
#include <kunit/mock.h>
#include "include/defex_config.h"
#include "include/defex_internal.h"

extern unsigned int global_features_status;

/* Helper methods for testing */

int get_current_ped_features(void)
{
	int ped_features = 0;

	ped_features |= (defex_get_features() & FEATURE_CHECK_CREDS);
	ped_features |= (defex_get_features() & FEATURE_CHECK_CREDS_SOFT);
	return ped_features;
}

int get_current_safeplace_features(void)
{
	int safeplace_features = 0;

	safeplace_features |= (defex_get_features() & FEATURE_SAFEPLACE);
	safeplace_features |= (defex_get_features() & FEATURE_SAFEPLACE_SOFT);
	return safeplace_features;
}

int get_current_immutable_features(void)
{
	int immutable_features = 0;

	immutable_features |= (defex_get_features() & FEATURE_IMMUTABLE);
	immutable_features |= (defex_get_features() & FEATURE_IMMUTABLE_SOFT);
	return immutable_features;
}

int get_current_immutable_root_features(void)
{
	int immutable_root_features = 0;

	immutable_root_features |= (defex_get_features() & FEATURE_IMMUTABLE_ROOT);
	immutable_root_features |= (defex_get_features() & FEATURE_IMMUTABLE_ROOT_SOFT);
	return immutable_root_features;
}

int get_current_integrity_features(void)
{
	int integrity_features = 0;

	integrity_features |= (defex_get_features() & FEATURE_INTEGRITY);
	integrity_features |= (defex_get_features() & FEATURE_INTEGRITY_SOFT);
	return integrity_features;
}

int get_current_trusted_map_features(void)
{
	int trusted_map_features = 0;

	trusted_map_features |= (defex_get_features() & FEATURE_TRUSTED_MAP);
	trusted_map_features |= (defex_get_features() & FEATURE_TRUSTED_MAP_SOFT);
	trusted_map_features |= (defex_get_features() & DEFEX_TM_DEBUG_VIOLATIONS);
	trusted_map_features |= (defex_get_features() & DEFEX_TM_DEBUG_CALLS);
	return trusted_map_features;
}

static void defex_get_mode_test(struct kunit *test)
{
	unsigned int expected_features;
	unsigned int current_features_backup;

#ifdef DEFEX_PED_ENABLE
	expected_features = 0;
	expected_features |= get_current_safeplace_features();
	expected_features |= get_current_immutable_features();
	expected_features |= get_current_integrity_features();
	expected_features |= get_current_trusted_map_features();
	expected_features |= get_current_immutable_root_features();

	current_features_backup = global_features_status;

	expected_features |= FEATURE_CHECK_CREDS;
	defex_set_features(expected_features);
	KUNIT_EXPECT_EQ(test, defex_get_features(), expected_features);

	expected_features |= FEATURE_CHECK_CREDS_SOFT;
	defex_set_features(expected_features);
	KUNIT_EXPECT_EQ(test, defex_get_features(), expected_features);

	global_features_status = current_features_backup;
#endif /* DEFEX_PED_ENABLE */
/*-------------------------------------------------------------------*/
#ifdef DEFEX_SAFEPLACE_ENABLE
	expected_features = 0;
	expected_features |= get_current_ped_features();
	expected_features |= get_current_immutable_features();
	expected_features |= get_current_integrity_features();
	expected_features |= get_current_trusted_map_features();
	expected_features |= get_current_immutable_root_features();

	current_features_backup = global_features_status;

	expected_features |= FEATURE_SAFEPLACE;
	defex_set_features(expected_features);
	KUNIT_EXPECT_EQ(test, defex_get_features(), expected_features);

	expected_features |= FEATURE_SAFEPLACE_SOFT;
	defex_set_features(expected_features);
	KUNIT_EXPECT_EQ(test, defex_get_features(), expected_features);

	global_features_status = current_features_backup;
#endif /* DEFEX_SAFEPLACE_ENABLE */
/*-------------------------------------------------------------------*/
#ifdef DEFEX_IMMUTABLE_ENABLE
	expected_features = 0;
	expected_features |= get_current_ped_features();
	expected_features |= get_current_safeplace_features();
	expected_features |= get_current_integrity_features();
	expected_features |= get_current_trusted_map_features();
	expected_features |= get_current_immutable_root_features();

	current_features_backup = global_features_status;

	expected_features |= FEATURE_IMMUTABLE;
	defex_set_features(expected_features);
	KUNIT_EXPECT_EQ(test, defex_get_features(), expected_features);

	expected_features |= FEATURE_IMMUTABLE_SOFT;
	defex_set_features(expected_features);
	KUNIT_EXPECT_EQ(test, defex_get_features(), expected_features);

	global_features_status = current_features_backup;
#endif /* DEFEX_IMMUTABLE_ENABLE */
/*-------------------------------------------------------------------*/
#ifdef DEFEX_INTEGRITY_ENABLE
	expected_features = 0;
	expected_features |= get_current_ped_features();
	expected_features |= get_current_safeplace_features();
	expected_features |= get_current_immutable_features();
	expected_features |= get_current_trusted_map_features();
	expected_features |= get_current_immutable_root_features();

	current_features_backup = global_features_status;

	expected_features |= FEATURE_INTEGRITY;
	defex_set_features(expected_features);
	KUNIT_EXPECT_EQ(test, defex_get_features(), expected_features);

	expected_features |= FEATURE_INTEGRITY_SOFT;
	defex_set_features(expected_features);
	KUNIT_EXPECT_EQ(test, defex_get_features(), expected_features);

	global_features_status = current_features_backup;
#endif /* DEFEX_INTEGRITY_ENABLE */
	/*-------------------------------------------------------------------*/
#ifdef DEFEX_TRUSTED_MAP_ENABLE
	expected_features = 0;
	expected_features |= get_current_ped_features();
	expected_features |= get_current_safeplace_features();
	expected_features |= get_current_immutable_features();
	expected_features |= get_current_integrity_features();
	expected_features |= get_current_immutable_root_features();
	expected_features |= (defex_get_features() & DEFEX_TM_DEBUG_VIOLATIONS);
	expected_features |= (defex_get_features() & DEFEX_TM_DEBUG_CALLS);

	current_features_backup = global_features_status;

	expected_features |= FEATURE_TRUSTED_MAP;
	defex_set_features(expected_features);
	KUNIT_EXPECT_EQ(test, defex_get_features(), expected_features);

	expected_features |= FEATURE_TRUSTED_MAP_SOFT;
	defex_set_features(expected_features);
	KUNIT_EXPECT_EQ(test, defex_get_features(), expected_features);

	global_features_status = current_features_backup;
#endif /* DEFEX_TRUSTED_MAP_ENABLE */
	KUNIT_SUCCEED(test);
}

static int defex_config_test_init(struct kunit *test)
{
	return 0;
}

static void defex_config_test_exit(struct kunit *test)
{
}

static struct kunit_case defex_config_test_cases[] = {
	/* TEST FUNC DEFINES */
	KUNIT_CASE(defex_get_mode_test),
	{},
};

static struct kunit_suite defex_config_test_module = {
	.name = "defex_config_test",
	.init = defex_config_test_init,
	.exit = defex_config_test_exit,
	.test_cases = defex_config_test_cases,
};
kunit_test_suites(&defex_config_test_module);

MODULE_LICENSE("GPL v2");
