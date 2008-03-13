/*
	Robert Umbehant <rumbehant@wheresjames.com> lifted this code from 
	http://autopackage.org/docs/binreloc/ , slightly mutilated it to
	compile with a c++ compiler, and thereafter, burdened the public
	domain with his changes.
	
	Thanks Hongli!!!
*/

/*** LICENSE BEGIN */
/*
 * BinReloc - a library for creating relocatable executables
 * Written by: Hongli Lai <h.lai@chello.nl>
 * http://autopackage.org/
 *
 * This source code is public domain. You can relicense this code
 * under whatever license you want.
 *
 * See http://autopackage.org/docs/binreloc/ for
 * more information and how to use this.
 */
/*** LICENSE END */

#include "binreloc.h"

static char *exe = (char *) NULL;

/** Initialize the BinReloc library (for applications).
 *
 * This function must be called before using any other BinReloc functions.
 * It attempts to locate the application's canonical filename.
 *
 * @note If you want to use BinReloc for a library, then you should call
 *       br_init_lib() instead.
 *
 * @param error  If BinReloc failed to initialize, then the error code will
 *               be stored in this variable. Set to NULL if you want to
 *               ignore this. See #BrInitError for a list of error codes.
 *
 * @returns 1 on success, 0 if BinReloc failed to initialize.
 */
int
br_init (BrInitError *error)
{
	exe = _br_find_exe (error);
	return exe != NULL;
}


/** Initialize the BinReloc library (for libraries).
 *
 * This function must be called before using any other BinReloc functions.
 * It attempts to locate the calling library's canonical filename.
 *
 * @note The BinReloc source code MUST be included in your library, or this
 *       function won't work correctly.
 *
 * @param error  If BinReloc failed to initialize, then the error code will
 *               be stored in this variable. Set to NULL if you want to
 *               ignore this. See #BrInitError for a list of error codes.
 *
 * @returns 1 on success, 0 if a filename cannot be found.
 */
int
br_init_lib (BrInitError *error)
{
	exe = _br_find_exe_for_symbol ((const void *) "", error);
	return exe != NULL;
}


/** Find the canonical filename of the current application.
 *
 * @param default_exe  A default filename which will be used as fallback.
 * @returns A string containing the application's canonical filename,
 *          which must be freed when no longer necessary. If BinReloc is
 *          not initialized, or if br_init() failed, then a copy of
 *          default_exe will be returned. If default_exe is NULL, then
 *          NULL will be returned.
 */
char *
br_find_exe (const char *default_exe)
{
	if (exe == (char *) NULL) {
		/* BinReloc is not initialized. */
		if (default_exe != (const char *) NULL)
			return strdup (default_exe);
		else
			return (char *) NULL;
	}
	return strdup (exe);
}


/** Locate the directory in which the current application is installed.
 *
 * The prefix is generated by the following pseudo-code evaluation:
 * \code
 * dirname(exename)
 * \endcode
 *
 * @param default_dir  A default directory which will used as fallback.
 * @return A string containing the directory, which must be freed when no
 *         longer necessary. If BinReloc is not initialized, or if the
 *         initialization function failed, then a copy of default_dir
 *         will be returned. If default_dir is NULL, then NULL will be
 *         returned.
 */
char *
br_find_exe_dir (const char *default_dir)
{
	if (exe == NULL) {
		/* BinReloc not initialized. */
		if (default_dir != NULL)
			return strdup (default_dir);
		else
			return NULL;
	}

	return br_dirname (exe);
}


/** Locate the prefix in which the current application is installed.
 *
 * The prefix is generated by the following pseudo-code evaluation:
 * \code
 * dirname(dirname(exename))
 * \endcode
 *
 * @param default_prefix  A default prefix which will used as fallback.
 * @return A string containing the prefix, which must be freed when no
 *         longer necessary. If BinReloc is not initialized, or if
 *         the initialization function failed, then a copy of default_prefix
 *         will be returned. If default_prefix is NULL, then NULL will be returned.
 */
char *
br_find_prefix (const char *default_prefix)
{
	char *dir1, *dir2;

	if (exe == (char *) NULL) {
		/* BinReloc not initialized. */
		if (default_prefix != (const char *) NULL)
			return strdup (default_prefix);
		else
			return (char *) NULL;
	}

	dir1 = br_dirname (exe);
	dir2 = br_dirname (dir1);
	free (dir1);
	return dir2;
}


/** Locate the application's binary folder.
 *
 * The path is generated by the following pseudo-code evaluation:
 * \code
 * prefix + "/bin"
 * \endcode
 *
 * @param default_bin_dir  A default path which will used as fallback.
 * @return A string containing the bin folder's path, which must be freed when
 *         no longer necessary. If BinReloc is not initialized, or if
 *         the initialization function failed, then a copy of default_bin_dir will
 *         be returned. If default_bin_dir is NULL, then NULL will be returned.
 */
char *
br_find_bin_dir (const char *default_bin_dir)
{
	char *prefix, *dir;

	prefix = br_find_prefix ((const char *) NULL);
	if (prefix == (char *) NULL) {
		/* BinReloc not initialized. */
		if (default_bin_dir != (const char *) NULL)
			return strdup (default_bin_dir);
		else
			return (char *) NULL;
	}

	dir = br_build_path (prefix, "bin");
	free (prefix);
	return dir;
}


/** Locate the application's superuser binary folder.
 *
 * The path is generated by the following pseudo-code evaluation:
 * \code
 * prefix + "/sbin"
 * \endcode
 *
 * @param default_sbin_dir  A default path which will used as fallback.
 * @return A string containing the sbin folder's path, which must be freed when
 *         no longer necessary. If BinReloc is not initialized, or if the
 *         initialization function failed, then a copy of default_sbin_dir will
 *         be returned. If default_bin_dir is NULL, then NULL will be returned.
 */
char *
br_find_sbin_dir (const char *default_sbin_dir)
{
	char *prefix, *dir;

	prefix = br_find_prefix ((const char *) NULL);
	if (prefix == (char *) NULL) {
		/* BinReloc not initialized. */
		if (default_sbin_dir != (const char *) NULL)
			return strdup (default_sbin_dir);
		else
			return (char *) NULL;
	}

	dir = br_build_path (prefix, "sbin");
	free (prefix);
	return dir;
}


/** Locate the application's data folder.
 *
 * The path is generated by the following pseudo-code evaluation:
 * \code
 * prefix + "/share"
 * \endcode
 *
 * @param default_data_dir  A default path which will used as fallback.
 * @return A string containing the data folder's path, which must be freed when
 *         no longer necessary. If BinReloc is not initialized, or if the
 *         initialization function failed, then a copy of default_data_dir
 *         will be returned. If default_data_dir is NULL, then NULL will be
 *         returned.
 */
char *
br_find_data_dir (const char *default_data_dir)
{
	char *prefix, *dir;

	prefix = br_find_prefix ((const char *) NULL);
	if (prefix == (char *) NULL) {
		/* BinReloc not initialized. */
		if (default_data_dir != (const char *) NULL)
			return strdup (default_data_dir);
		else
			return (char *) NULL;
	}

	dir = br_build_path (prefix, "share");
	free (prefix);
	return dir;
}


/** Locate the application's localization folder.
 *
 * The path is generated by the following pseudo-code evaluation:
 * \code
 * prefix + "/share/locale"
 * \endcode
 *
 * @param default_locale_dir  A default path which will used as fallback.
 * @return A string containing the localization folder's path, which must be freed when
 *         no longer necessary. If BinReloc is not initialized, or if the
 *         initialization function failed, then a copy of default_locale_dir will be returned.
 *         If default_locale_dir is NULL, then NULL will be returned.
 */
char *
br_find_locale_dir (const char *default_locale_dir)
{
	char *data_dir, *dir;

	data_dir = br_find_data_dir ((const char *) NULL);
	if (data_dir == (char *) NULL) {
		/* BinReloc not initialized. */
		if (default_locale_dir != (const char *) NULL)
			return strdup (default_locale_dir);
		else
			return (char *) NULL;
	}

	dir = br_build_path (data_dir, "locale");
	free (data_dir);
	return dir;
}


/** Locate the application's library folder.
 *
 * The path is generated by the following pseudo-code evaluation:
 * \code
 * prefix + "/lib"
 * \endcode
 *
 * @param default_lib_dir  A default path which will used as fallback.
 * @return A string containing the library folder's path, which must be freed when
 *         no longer necessary. If BinReloc is not initialized, or if the initialization
 *         function failed, then a copy of default_lib_dir will be returned.
 *         If default_lib_dir is NULL, then NULL will be returned.
 */
char *
br_find_lib_dir (const char *default_lib_dir)
{
	char *prefix, *dir;

	prefix = br_find_prefix ((const char *) NULL);
	if (prefix == (char *) NULL) {
		/* BinReloc not initialized. */
		if (default_lib_dir != (const char *) NULL)
			return strdup (default_lib_dir);
		else
			return (char *) NULL;
	}

	dir = br_build_path (prefix, "lib");
	free (prefix);
	return dir;
}


/** Locate the application's libexec folder.
 *
 * The path is generated by the following pseudo-code evaluation:
 * \code
 * prefix + "/libexec"
 * \endcode
 *
 * @param default_libexec_dir  A default path which will used as fallback.
 * @return A string containing the libexec folder's path, which must be freed when
 *         no longer necessary. If BinReloc is not initialized, or if the initialization
 *         function failed, then a copy of default_libexec_dir will be returned.
 *         If default_libexec_dir is NULL, then NULL will be returned.
 */
char *
br_find_libexec_dir (const char *default_libexec_dir)
{
	char *prefix, *dir;

	prefix = br_find_prefix ((const char *) NULL);
	if (prefix == (char *) NULL) {
		/* BinReloc not initialized. */
		if (default_libexec_dir != (const char *) NULL)
			return strdup (default_libexec_dir);
		else
			return (char *) NULL;
	}

	dir = br_build_path (prefix, "libexec");
	free (prefix);
	return dir;
}


/** Locate the application's configuration files folder.
 *
 * The path is generated by the following pseudo-code evaluation:
 * \code
 * prefix + "/etc"
 * \endcode
 *
 * @param default_etc_dir  A default path which will used as fallback.
 * @return A string containing the etc folder's path, which must be freed when
 *         no longer necessary. If BinReloc is not initialized, or if the initialization
 *         function failed, then a copy of default_etc_dir will be returned.
 *         If default_etc_dir is NULL, then NULL will be returned.
 */
char *
br_find_etc_dir (const char *default_etc_dir)
{
	char *prefix, *dir;

	prefix = br_find_prefix ((const char *) NULL);
	if (prefix == (char *) NULL) {
		/* BinReloc not initialized. */
		if (default_etc_dir != (const char *) NULL)
			return strdup (default_etc_dir);
		else
			return (char *) NULL;
	}

	dir = br_build_path (prefix, "etc");
	free (prefix);
	return dir;
}


/***********************
 * Utility functions
 ***********************/

/** Concatenate str1 and str2 to a newly allocated string.
 *
 * @param str1 A string.
 * @param str2 Another string.
 * @returns A newly-allocated string. This string should be freed when no longer needed.
 */
char *
br_strcat (const char *str1, const char *str2)
{
	char *result;
	size_t len1, len2;

	if (str1 == NULL)
		str1 = "";
	if (str2 == NULL)
		str2 = "";

	len1 = strlen (str1);
	len2 = strlen (str2);

	result = (char *) malloc (len1 + len2 + 1);
	memcpy (result, str1, len1);
	memcpy (result + len1, str2, len2);
	result[len1 + len2] = '\0';

	return result;
}


char *
br_build_path (const char *dir, const char *file)
{
	char *dir2, *result;
	size_t len;
	int must_free = 0;

	len = strlen (dir);
	if (len > 0 && dir[len - 1] != '/') {
		dir2 = br_strcat (dir, "/");
		must_free = 1;
	} else
		dir2 = (char *) dir;

	result = br_strcat (dir2, file);
	if (must_free)
		free (dir2);
	return result;
}


/* Emulates glibc's strndup() */
static char *
br_strndup (const char *str, size_t size)
{
	char *result = (char *) NULL;
	size_t len;

	if (str == (const char *) NULL)
		return (char *) NULL;

	len = strlen (str);
	if (len == 0)
		return strdup ("");
	if (size > len)
		size = len;

	result = (char *) malloc (len + 1);
	memcpy (result, str, size);
	result[size] = '\0';
	return result;
}


/** Extracts the directory component of a path.
 *
 * Similar to g_dirname() or the dirname commandline application.
 *
 * Example:
 * \code
 * br_dirname ("/usr/local/foobar");  --> Returns: "/usr/local"
 * \endcode
 *
 * @param path  A path.
 * @returns     A directory name. This string should be freed when no longer needed.
 */
char *
br_dirname (const char *path)
{
	char *end, *result;

	if (path == (const char *) NULL)
		return (char *) NULL;

	end = strrchr (path, '/');
	if (end == (const char *) NULL)
		return strdup (".");

	while (end > path && *end == '/')
		end--;
	result = br_strndup (path, end - path + 1);
	if (result[0] == 0) {
		free (result);
		return strdup ("/");
	} else
		return result;
}
