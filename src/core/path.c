/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/core/path.h>
#include <pure64/core/error.h>
#include <pure64/core/memory.h>
#include <pure64/core/string.h>

static int is_separator(char c) {
	if ((c == '/') || (c == '\\'))
		return 1;
	else
		return 0;
}

void pure64_path_init(struct pure64_path *path) {
	path->name_array = pure64_null;
	path->name_count = 0;
}

void pure64_path_free(struct pure64_path *path) {

	pure64_uint64 i;

	for (i = 0; i < path->name_count; i++)
		pure64_free(path->name_array[i].data);

	pure64_free(path->name_array);

	path->name_array = pure64_null;
	path->name_count = 0;
}

const char * pure64_path_get_name(const struct pure64_path *path,
                                  pure64_uint64 index) {

	if (index >= path->name_count)
		return pure64_null;

	return path->name_array[index].data;
}

pure64_uint64 pure64_path_get_name_count(const struct pure64_path *path) {

	return path->name_count;
}

int pure64_path_normalize(struct pure64_path *path) {

	pure64_uint64 i;
	pure64_uint64 j;

	i = 0;

	while (i < path->name_count) {

		if (path->name_array[i].data == pure64_null) {

			return PURE64_EFAULT;

		} else if (pure64_strcmp(path->name_array[i].data, ".") == 0) {

			pure64_free(path->name_array[i].data);

			for (j = i + 1; j < path->name_count; j++) {
				path->name_array[j - 1] = path->name_array[j];
			}

			path->name_count--;

		} else if (pure64_strcmp(path->name_array[i].data, "..") == 0) {

			pure64_free(path->name_array[i].data);

			if (i == 0) {
				path->name_count--;
				continue;
			}

			i--;

			pure64_free(path->name_array[i].data);

			for (j = i + 2; j < path->name_count; j++)
				path->name_array[j - 2] = path->name_array[j];

			path->name_count -= 2;

		} else {
			i++;
			continue;
		}
	}

	return 0;
}

int pure64_path_parse(struct pure64_path *path,
                      const char *path_str) {

	int err;
	pure64_uint64 i;
	char *tmp;
	char *tmp2;
	pure64_uint64 tmp_size;
	pure64_uint64 tmp_res;

	tmp = pure64_null;
	tmp_size = 0;
	tmp_res = 0;

	for (i = 0; path_str[i]; i++) {
		if (is_separator(path_str[i])) {

			if (tmp_size == 0)
				continue;

			err = pure64_path_push_child(path, tmp);
			if (err != 0) {
				pure64_free(tmp);
				return err;
			}

			tmp_size = 0;

		} else {

			if ((tmp_size + 1) >= tmp_res) {
				tmp_res += 64;
				tmp2 = pure64_realloc(tmp, tmp_res);
				if (tmp2 == pure64_null) {
					pure64_free(tmp);
					return PURE64_ENOMEM;
				}
				tmp = tmp2;
			}

			tmp[tmp_size++] = path_str[i];
			tmp[tmp_size] = 0;
		}
	}

	if (tmp_size > 0) {
		err = pure64_path_push_child(path, tmp);
		if (err != 0) {
			pure64_free(tmp);
			return err;
		}
	}

	pure64_free(tmp);

	return 0;
}

int pure64_path_push_child(struct pure64_path *path,
                           const char *name) {

	char *tmp_name;
	pure64_uint64 name_size;
	struct pure64_path_name *name_array;
	pure64_uint64 name_array_size;

	name_array = path->name_array;

	name_array_size = path->name_count + 1;
	name_array_size *= sizeof(path->name_array[0]);

	name_array = pure64_realloc(name_array, name_array_size);
	if (name_array == pure64_null)
		return PURE64_ENOMEM;

	path->name_array = name_array;

	name_size = pure64_strlen(name);

	tmp_name = pure64_malloc(name_size + 1);
	if (tmp_name == pure64_null)
		return PURE64_ENOMEM;

	pure64_memcpy(tmp_name, name, name_size);

	tmp_name[name_size] = 0;

	path->name_array[path->name_count].data = tmp_name;
	path->name_array[path->name_count].size = name_size;

	path->name_count++;

	return 0;
}
