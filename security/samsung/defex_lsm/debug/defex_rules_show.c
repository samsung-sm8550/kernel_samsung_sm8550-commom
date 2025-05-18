// SPDX-License-Identifier: GPL-2.0
#include <linux/kobject.h>

#include "include/defex_debug.h"
#include "include/defex_rules.h"
#include "include/defex_tree.h"

const char header_name[16] = {"DEFEX_RULES_FILE"};
struct rule_item_struct *defex_packed_rules;
static char work_path[512];
static int packfiles_size, global_data_size;


const struct feature_match_entry feature_match[] = {
	{"feature_safeplace_path", feature_safeplace_path},
	{"feature_ped_exception", feature_ped_exception},
	{"feature_immutable_path_open", feature_immutable_path_open},
	{"feature_immutable_path_write", feature_immutable_path_write},
	{"feature_immutable_root", feature_immutable_root},
	{"feature_immutable_src_exception", feature_immutable_src_exception},
	{"feature_immutable_dst_exception", feature_immutable_dst_exception},
	{"feature_umhbin_path", feature_umhbin_path},
	{"feature_integrity_check", feature_integrity_check},
};

void feature_to_string(char *str, unsigned int flags)
{
	int i;

	str[0] = 0;
	for (i = 0; i < ARRAY_SIZE(feature_match); i++)
		if (flags & feature_match[i].feature_num) {
			if (str[0])
				strcat(str, ", ");
			strcat(str, feature_match[i].feature_name);
		}
	if (flags & feature_for_recovery) {
		if (str[0])
			strcat(str, ", ");
		strcat(str, "feature_for_recovery");
	}
}

static int check_array_size(struct rule_item_struct *ptr)
{
	unsigned long offset = (unsigned long)ptr - (unsigned long)defex_packed_rules;
	int min_size = (global_data_size < packfiles_size)?global_data_size:packfiles_size;

	offset += sizeof(struct rule_item_struct);

	if (offset > min_size)
		return 1;

	offset += ptr->size;
	if (offset > min_size)
		return 2;
	return 0;
}

static int parse_items(struct d_tree_item *base, size_t path_length, int level)
{
	struct d_tree_item tmp_item, *child_item;
	const char *subdir_ptr;
	unsigned int subdir_size;
	static char feature_list[128];
	int err, ret = 0;

	if (level > 8) {
		defex_log_timeoff("Level is too deep");
		return -1;
	}

	if (path_length > (sizeof(work_path) - 128)) {
		defex_log_timeoff("Work path is too long");
		return -1;
	}

	child_item = d_tree_lookup_dir_init(base, &tmp_item);

	while (child_item) {
		subdir_size = 0;
		subdir_ptr = d_tree_get_subpath(child_item, &subdir_size);
		if (subdir_ptr)
			memcpy(work_path + path_length, subdir_ptr, (size_t)subdir_size);
		subdir_size += (unsigned int)path_length;
		work_path[subdir_size] = 0;

		if (child_item->features & active_feature_mask) {
			feature_to_string(feature_list, child_item->features);
			defex_log_blob("%s%c - %s", work_path,
				((child_item->features & feature_is_file)?' ':'/'),
				feature_list);
		}
		work_path[subdir_size++] = '/';
		work_path[subdir_size] = 0;
		err = parse_items(child_item, (size_t)subdir_size, level + 1);
		if (err != 0)
			return err;

		work_path[path_length] = 0;
		child_item = d_tree_next_child(base, child_item);
	}
	return ret;
}

int defex_rules_show(void *packed_rules, size_t rules_size)
{
	struct d_tree extern_tree;
	struct d_tree_item base;
	int res;

	work_path[0] = '/';
	work_path[1] = 0;

	if (d_tree_get_header(packed_rules, rules_size, &extern_tree)) {
		defex_log_timeoff("ERROR: Unknown file version!");
		return -1;
	}

	packfiles_size = (unsigned int)rules_size;
	global_data_size = extern_tree.data_size;

	defex_log_timeoff("Rules binary size: %d", packfiles_size);
	defex_log_timeoff("Rules internal data size: %d", global_data_size);

	if (global_data_size > packfiles_size)
		defex_log_timeoff("%s%s", "WARNING: Internal size is bigger than binary size,",
			" possible structure error!");

	defex_log_timeoff("File List:");
	if (!d_tree_get_item_header(&extern_tree, 0, &base))
		return -1;

	res = parse_items(&base, 1, 1);
	defex_log_timeoff("== End of File List ==");
	return res;
}
