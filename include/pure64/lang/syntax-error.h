/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_LANG_SYNTAX_ERROR_H
#define PURE64_LANG_SYNTAX_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/** A syntax error, caused by
 * a mistake in the configuration file.
 * @ingroup lang-api
 * */

struct pure64_syntax_error {
	/** The name of the source that
	 * the error originated from. This
	 * is usually a filename or @ref pure64_null. */
	const char *source;
	/** A description of the error that
	 * occurred. */
	const char *desc;
	/** The line that the error was found at. */
	unsigned long int line;
	/** The column that the error was found at. */
	unsigned long int column;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PURE64_LANG_SYNTAX_ERROR_H */
