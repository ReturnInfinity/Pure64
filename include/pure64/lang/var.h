/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_LANG_VAR_H
#define PURE64_LANG_VAR_H

#include <pure64/core/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pure64_scanner;
struct pure64_syntax_error;
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

/** Represents a variable value.
 * @ingroup lang-api
 * */

struct pure64_value {
	/** The type of variable value. */
	enum pure64_type type;
	/** The line that the value begins at. */
	unsigned int line;
	/** The column that the value begins at. */
	unsigned int column;
	/** The type-specific value data. */
	union {
		/** Object data, if the value is an object. */
		struct pure64_object object;
		/** List data, if the value is a list. */
		struct pure64_list list;
		/** String data, if the value is a string. */
		char *string;
		/** The number value, if the value is a number. */
		pure64_uint64 number;
		/** The boolean value, if the value is boolean. */
		pure64_bool _bool;
	} u;
};

/** Initializes the value structure.
 * @param value The value structure to initialize.
 * @ingroup lang-api
 * */

void pure64_value_init(struct pure64_value *value);

/** Releases memory allocated by the value structure.
 * @param value An initialized value structure.
 * @ingroup lang-api
 * */

void pure64_value_done(struct pure64_value *value);

/** Copies a variable value.
 * @param dst The destination value.
 * This is where the copied data will go to.
 * @param src The source value.
 * This is where the data is copied from.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_value_copy(struct pure64_value *dst,
                      const struct pure64_value *src);

/** This is a variable key, used to
 * distinguish variables.
 * @ingroup lang-api
 * */

struct pure64_key {
	/** The ID of the variable. */
	const char *id;
	/** The number of characters in the ID. */
	pure64_size id_size;
	/** The line that the key started on. */
	unsigned int line;
	/** The column that the key started on. */
	unsigned int column;
};

/** Initializes a variable key for use.
 * @param key An uninitialized variable key.
 * @ingroup lang-api
 * */

void pure64_key_init(struct pure64_key *key);

/** Releases memory allocated by a variable key.
 * @param key An initialized variable key.
 * @ingroup lang-api
 * */

void pure64_key_done(struct pure64_key *key);

/** Compares the ID of the key to
 * another ID in a null-terminated string.
 * @param key An initialized key structure.
 * @param id The string ID to compare with
 * the key ID.
 * @returns One if the key ID is greater than
 * the string ID, negative one if the key ID
 * is less than the string ID, zero if they're equal.
 * @ingroup lang-api
 * */

int pure64_key_cmp_id(struct pure64_key *key,
                      const char *id);

/** Parses a variable key.
 * @param key An initialized key structure.
 * @param scanner An initialized scanner structure.
 * @param error An error structure, in case there
 * is a syntax error.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_key_parse(struct pure64_key *key,
                     struct pure64_scanner *scanner,
                     struct pure64_syntax_error *error);

/** Parses a variable key, from a null-terminated string.
 * @param key An initialized key structure.
 * @param source A null-terminated source containing the key.
 * @param error An error structure in case there is a syntax error.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_key_parse_s(struct pure64_key *key,
                       const char *source,
                       struct pure64_syntax_error *error);

/** Used to contain a variable found
 * in a configuration file.
 * @ingroup lang-api
 * */

struct pure64_var {
	/** The name of the variable. */
	struct pure64_key key;
	/** The value of the variable. */
	struct pure64_value value;
};

/** Initializes a variable structure.
 * @param var The variable structure to initialize.
 * @ingroup lang-api
 * */

void pure64_var_init(struct pure64_var *var);

/** Releases memory allocated by a variable.
 * @param var An initialized variable structure.
 * @ingroup lang-api
 * */

void pure64_var_done(struct pure64_var *var);

/** Parses a variable.
 * @param var An initialized variable structure.
 * @param scanner An initialized scanner structure.
 * @param error An error structure used to relay information
 * on where a syntax error has occured. May be @ref pure64_null.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_var_parse(struct pure64_var *var,
                     struct pure64_scanner *scanner,
                     struct pure64_syntax_error *error);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PURE64_LANG_VAR_H */
