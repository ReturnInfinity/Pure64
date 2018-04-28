/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_LANG_VAR_H
#define PURE64_LANG_VAR_H

#include <pure64/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct pure64_token;
struct pure64_var;
struct pure64_value;

/** Enumerates a data type.
 * This is relevant in determining
 * the value type of a variable.
 * @ingroup lang-api
 * */

enum pure64_type {
	/** Indicates that there is no type. */
	PURE64_TYPE_null,
	/** Indicates that the value is a string. */
	PURE64_TYPE_string,
	/** Indicates that the value is a number. */
	PURE64_TYPE_number,
	/** Indicates that the value is boolean. */
	PURE64_TYPE_boolean,
	/** Indicates that the value is a list. */
	PURE64_TYPE_list,
	/** Indicates that the value is an object. */
	PURE64_TYPE_object
};

/** A list of values, contained by a variable.
 * @ingroup lang-api
 * */

struct pure64_list {
	/** A pointer to the value array. */
	struct pure64_value *value_array;
	/** The number of values in the value array. */
	pure64_size value_count;
};

/** Initializes a list structure.
 * @param list An uninitialized list structure.
 * @ingroup lang-api
 * */

void pure64_list_init(struct pure64_list *list);

/** Releases memory allocated by a list structure.
 * @param list An initialized list structure.
 * @ingroup lang-api
 * */

void pure64_list_done(struct pure64_list *list);

/** Copies a list structure.
 * @param dst The destination list structure.
 * This is the list that receives the copied data.
 * @param src The source list structure.
 * This is the list that gets copied.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_list_copy(struct pure64_list *dst,
                     const struct pure64_list *src);

/** Pushes a value to the end of the list.
 * @param list An initialized list structure.
 * @param value The value to push to the list.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_list_push(struct pure64_list *list,
                     const struct pure64_value *value);

/** Represents an object structure contained
 * within a configuration file. It contains a
 * series of variables, which can be objects
 * themselves.
 * @ingroup lang-api
 * */

struct pure64_object {
	/** An array of variables contained by the object. */
	struct pure64_var *var_array;
	/** The number of variables in the variable array. */
	pure64_size var_count;
};

/** Initializes an object structure.
 * @param object The object structure to initialize.
 * @ingroup lang-api
 * */

void pure64_object_init(struct pure64_object *object);

/** Releases memory allocated by the object structure.
 * @param object An initialized object structure.
 * @ingroup lang-api
 * */

void pure64_object_done(struct pure64_object *object);

/** Copies an object structure.
 * @param dst The destination object structure.
 * This is the object structure that receives the copied data.
 * @param src The source object structure.
 * This is the object structure that is copied.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_object_copy(struct pure64_object *dst,
                       const struct pure64_object *src);

/** Pushes a variable to the end of the object structure.
 * @param object An initialized object structure.
 * @param var The variable to push to the end of the object.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_object_push(struct pure64_object *object,
                       struct pure64_var *var);

struct pure64_value {
	enum pure64_type type;
	unsigned int line;
	unsigned int column;
	union {
		struct pure64_object object;
		struct pure64_list list;
		char *string;
		pure64_uint64 number;
		pure64_bool _bool;
	} u;
};

void pure64_value_init(struct pure64_value *value);

void pure64_value_done(struct pure64_value *value);

int pure64_value_copy(struct pure64_value *dst,
                      const struct pure64_value *src);

struct pure64_key {
	char *id;
	pure64_size id_size;
	unsigned int line;
	unsigned int column;
};

void pure64_key_init(struct pure64_key *key);

void pure64_key_done(struct pure64_key *key);

/** Used to contain a variable found
 * in a configuration file.
 * @ingroup lang-api
 * */

struct pure64_var
{
	/** The name of the variable. */
	struct pure64_key key;
	/** The value of the variable. */
	struct pure64_value value;
};

void pure64_var_init(struct pure64_var *var);

void pure64_var_done(struct pure64_var *var);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PURE64_LANG_VAR_H */
