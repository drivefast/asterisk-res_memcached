/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 2010, Radu M
 *
 * Radu Maierean <radu dot maierean at g-mail>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 *
 * Please follow coding guidelines 
 * http://svn.digium.com/view/asterisk/trunk/doc/CODING-GUIDELINES
 */

/*! \file
 *
 * \brief MCD() memcache get/set value for key
 * \brief mcdget memcache get value for key
 * \brief mcdset memcache set key to value
 * \brief mcdadd memcache add
 * \brief mcdreplace memcache replace
 * \brief mcdappend memcache append to string variable
 * \brief mcddelete memcache delete
 * \brief MCDCOUNTER() memcache numeric counter set, test and increment/decrement
 *
 * \author\verbatim Radu Maierean <radu dot maierean at gmail> \endverbatim
 * 
 * \ingroup applications
 */

/*** MODULEINFO
	<defaultenabled>yes</defaultenabled>
    <depend>memcached</depend>
 ***/

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision: 200656 $")

#include "asterisk/file.h"
#include "asterisk/channel.h"
#include "asterisk/pbx.h"
#include "asterisk/module.h"
#include "asterisk/app.h"
#include "asterisk/utils.h"

#include <stdlib.h>
#include <libmemcached-1.0/memcached.h>
#include <libmemcachedutil-1.0/util.h>

/*** DOCUMENTATION
	<function name="MCD" language="en_US">
		<synopsis>
			gets or sets the value for a key in the cache store
		</synopsis>	
		<syntax>
			<parameter name="key" required="true">
				<para>key to be looked up, or set</para>
			</parameter>
		</syntax>
		<description>
			<para>gets or sets the value for a key in the cache store. when used in write mode, 
			the function invokes the set memcached command.</para>
		</description>
		<see-also>
			<ref type="application">mcdadd</ref>
			<ref type="application">mcdreplace</ref>
			<ref type="application">mcddelete</ref>
		</see-also>
	</function>
	<application name="mcdget" language="en_US">
		<synopsis>
			stores the value of a key in the cache store in a dialplan variable
		</synopsis>
		<syntax>
			<parameter name="varname" required="true">
				<para>the name (not the contents!) of the variable to set</para>
			</parameter>
			<parameter name="key" required="true">
				<para>key to be looked up</para>
			</parameter>
		</syntax>
		<description>
			<para>stores the value of a key in the cache store in a dialplan variable.</para>
		</description>
		<see-also>
			<ref type="function">MCD</ref>
		</see-also>
	</application>
	<application name="mcdset" language="en_US">
		<synopsis>
			stores a value in the cache store, with the given key
		</synopsis>
		<syntax>
			<parameter name="key" required="true">
				<para>key to be looked up, or generated if missing</para>
			</parameter>
			<parameter name="value" required="true">
				<para>data to store</para>
			</parameter>
		</syntax>
		<description>
			<para>stores a value in the cache store, with the given key. if the key doesnt already 
			exist, it is added. the memcached server can auto-expire (and remove) the value after a 
			given amount of time.</para>
		</description>
		<see-also>
			<ref type="function">MCD</ref>
			<ref type="application">mcdadd</ref>
			<ref type="application">mcdreplace</ref>
			<ref type="application">mcddelete</ref>
		</see-also>
	</application>
	<application name="mcdadd" language="en_US">
		<synopsis>
			adds a value in the cache store, with the given key
		</synopsis>
		<syntax>
			<parameter name="key" required="true">
				<para>key to be used</para>
			</parameter>
			<parameter name="value" required="true">
				<para>data to store</para>
			</parameter>
		</syntax>
		<description>
			<para>adds a value in the cache store, with the given key. if the key already exists, 
			the operation will fail. the memcached server can auto-expire (and remove) the value 
			after a given amount of time set in the configuration file, or by the MCDTTL dialplan 
			variable.</para>
		</description>
		<see-also>
			<ref type="function">MCD</ref>
			<ref type="application">mcdreplace</ref>
			<ref type="application">mcddelete</ref>
		</see-also>
	</application>
	<application name="mcdreplace" language="en_US">
		<synopsis>
			replaces the cache store value for a given key
		</synopsis>
		<syntax>
			<parameter name="key" required="true">
				<para>key to be looked up</para>
			</parameter>
			<parameter name="value" required="true">
				<para>data to store</para>
			</parameter>
		</syntax>
		<description>
			<para>replaces the value in the cache store at the given key. if the key is missing, the 
			operation will fail. the memcached server can auto-expire (and remove) the value after 
			a given amount of time set in the configuration file, or by the MCDTTL dialplan 
			variable.</para>
		</description>
		<see-also>
			<ref type="function">MCD</ref>
			<ref type="application">mcdadd</ref>
			<ref type="application">mcddelete</ref>
		</see-also>
	</application>
	<application name="mcdappend" language="en_US">
		<synopsis>
			appends more text to an already existing value in the cache store 
		</synopsis>
		<syntax>
			<parameter name="key" required="true">
				<para>key to be looked up</para>
			</parameter>
			<parameter name="value" required="true">
				<para>text to append</para>
			</parameter>
		</syntax>
		<description>
			<para>appends more text to an already existing value in the cache store. if the key is 
			missing, the operation will fail. the operation is atomic, in the sense that you dont 
			have to get, then set the value and run the risk that another memcache client modified 
			the value in the mean time. the memcached server can auto-expire (and remove) the value 
			after a given amount of time set in the configuration file, or by the MCDTTL dialplan 
			variable.</para>
		</description>
		<see-also>
			<ref type="function">MCD</ref>
			<ref type="application">mcdset</ref>
		</see-also>
	</application>
	<application name="mcddelete" language="en_US">
		<synopsis>
			forecefully delete the value in the cache store at the given key
		</synopsis>
		<syntax>
			<parameter name="key" required="true">
				<para>key to be looked up, or generated if missing</para>
			</parameter>
		</syntax>
		<description>
			<para>forecefully delete the value in the cache store at the given key</para>
		</description>
		<see-also>
			<ref type="function">MCD</ref>
			<ref type="application">mcdadd</ref>
			<ref type="application">mcdreplace</ref>
		</see-also>
	</application>
	<function name="MCDCOUNTER" language="en_US">
		<synopsis>
			on write, creates and initializes a memcache counter; on read, gets the value of a
			counter in the cache store, after optionally incrementing or decrementing it.
		</synopsis>	
		<syntax>
			<parameter name="key" required="true">
				<para>key to be looked up</para>
			</parameter>
			<parameter name="increment">
				<para>(only on read) the value to increment the counter with; may be a positive
				integer, a negative, or 0 which means the counter value is returned without executing 
				an operation on ir</para>
			</parameter>
		</syntax>
		<description>
			<para>on write, creates and initializes a memcache counter. on read, gets the value of a
			counter in the cache store, after optionally incrementing or decrementing it with the 
			given value. by default, the counters have an unlimited lifetime. to set a time to live 
			for them, set the MCDTTL dialplan variable with the desired value (in seconds). 
			the function only works if the binary protocol is activated (see config file).</para>
		</description>
		<see-also>
			<ref type="application">mcddelete</ref>
		</see-also>
	</function>
 ***/

/*
STANDARD CONFIGURATION FILE (/etc/asterisk/memcache.conf)
=========================================================
[general]
ttl=0                                 ; sets the default time-to-live, in seconds, for the key-value entries added 
                                      ;   or modified in the cache store. default value is 0, which means the entries
                                      ;   will persist forever. (note that memcached does NOT store its database on 
                                      ;   a non-volatile support, so the entries will be lost anyway when the 
                                      ;   memcached server goes down.) the ttl value can be overridden in the dialplan 
                                      ;   by the MCDTTL dialplan variable.
;binary_proto=yes                     ; using binary protocol for conversation with server; default is yes. note that 
                                      ;   the MCDCOUNTER() function is not happy if the protocol is not binary
hash=default                          ; hashing mode (see libmemcached documentation); accepted values are default
                                      ;   (which is actually md5), md5, crc, fnv1_64, fnv1_64a, fnv1_32, fnv1_32a,
                                      ;   jenkins, hsieh, murmur. make sure whatever you select is actually
                                      ;   supported by the library ecosystem on your machine
keyprefix=                            ; whatever string you specify here is prepended to each key that is retrieved
                                      ;   or stored, so that you can create some sort of a "domain" for your
                                      ;   asterisk server
server=localhost:11211                ; multiple 'server=' entries will create a cluster of servers to connect to;
;server=memcache.server.com:11211     ;   each entry is in the form host[:port], host being a fqdn or an ip address,
                                      ;   the default memcached port is 11211. if no entries, the module will at
                                      ;   least attempt to connect to a memcached running on the localhost

UNIT TESTING (using a dialplan macro)
=====================================
[macro-mcdtest]
exten => s,1,noop(>>>> performing memcached tests)
exten => s,n,answer()
exten => s,n,set(MCD(wrtest)=hello)
exten => s,n,noop(>>>> test 1 (write / read): '${MCD(wrtest)}' == 'hello')
exten => s,n,mcdappend(wrtest, world!)
exten => s,n,noop(>>>> test 2 (append): '${MCD(wrtest)}' == 'hello world!')
exten => s,n,mcdadd(wrtest,something)
exten => s,n,noop(>>>> test 3 (add failure): error ${MCDRESULT} == 14)
exten => s,n,mcdreplace(wrtest,goodbye world!)
exten => s,n,noop(>>>> test 4 (replace): '${MCD(wrtest)}' == 'goodbye world!')
exten => s,n,mcddelete(wrtest)
exten => s,n,set(testresult=${MCD(wrtest)})
exten => s,n,noop(>>>> test 5 (delete + get failure): error: ${MCDRESULT} == 16)
exten => s,n,set(MCDTTL=1)
exten => s,n,mcdset(wrtest,hello again)
exten => s,n,noop(>>>> test 6 (write w timeout): '${MCD(wrtest)}' == 'hello again')
exten => s,n,wait(2)
exten => s,n,set(testresult=${MCD(wrtest)})
exten => s,n,noop(>>>> test 7 (failure to read a timeout value): error ${MCDRESULT} == 16)
exten => s,n,set(MCDCOUNTER(counter)=678)
exten => s,n,noop(>>>> test 8 (counter set & readout): ${MCDCOUNTER(counter)})
exten => s,n,noop(>>>> test 9 (counter decrement by 12): ${MCDCOUNTER(counter,-12)})
exten => s,n,wait(2)
exten => s,n,noop(>>>> test 10 (counter expiration): ${MCDCOUNTER(counter)} / error: ${MCDRESULT})
exten => s,n,hangup()
*/

static char *app_mcdget =         "mcdget";
static char *app_mcdset =         "mcdset";
static char *app_mcdadd =         "mcdadd";
static char *app_mcdreplace =     "mcdreplace";
static char *app_mcdappend =      "mcdappend";
static char *app_mcddelete =      "mcddelete";

#define CONFIG_FILE_NAME          "memcached.conf"
#define MAX_ASTERISK_VARLEN       4096

// memcache properties
struct timespec to;
memcached_pool_st *mcdpool;
char keyprefix[65];
static int use_binary_proto;
static unsigned int mcdttl;

/* 
  // returned errors in the MCDRESULT variable:
  MEMCACHED_SUCCESS = 0,
  MEMCACHED_FAILURE = 1,
  MEMCACHED_HOST_LOOKUP_FAILURE = 2,
  MEMCACHED_CONNECTION_FAILURE = 3,
  MEMCACHED_CONNECTION_BIND_FAILURE = 4,
  MEMCACHED_WRITE_FAILURE = 5,
  MEMCACHED_READ_FAILURE = 6,
  MEMCACHED_UNKNOWN_READ_FAILURE = 7,
  MEMCACHED_PROTOCOL_ERROR = 8,
  MEMCACHED_CLIENT_ERROR = 9,
  MEMCACHED_SERVER_ERROR = 10,
  MEMCACHED_CONNECTION_SOCKET_CREATE_FAILURE = 11,
  MEMCACHED_DATA_EXISTS = 12,
  MEMCACHED_DATA_DOES_NOT_EXIST = 13,
  MEMCACHED_NOTSTORED = 14,
  MEMCACHED_STORED = 15,
  MEMCACHED_NOTFOUND = 16,
  MEMCACHED_MEMORY_ALLOCATION_FAILURE = 17,
  MEMCACHED_PARTIAL_READ = 18,
  MEMCACHED_SOME_ERRORS = 19,
  MEMCACHED_NO_SERVERS = 20,
  MEMCACHED_END = 21,
  MEMCACHED_DELETED = 22,
  MEMCACHED_VALUE = 23,
  MEMCACHED_STAT = 24,
  MEMCACHED_ITEM = 25,
  MEMCACHED_ERRNO = 26,
  MEMCACHED_FAIL_UNIX_SOCKET = 27,
  MEMCACHED_NOT_SUPPORTED = 28,
  MEMCACHED_NO_KEY_PROVIDED = 29, // Deprecated. Use MEMCACHED_BAD_KEY_PROVIDED! 
  MEMCACHED_FETCH_NOTFINISHED = 30,
  MEMCACHED_TIMEOUT = 31,
  MEMCACHED_BUFFERED = 32,
  MEMCACHED_BAD_KEY_PROVIDED = 33,
  MEMCACHED_INVALID_HOST_PROTOCOL = 34,
  MEMCACHED_SERVER_MARKED_DEAD = 35,
  MEMCACHED_UNKNOWN_STAT_KEY = 36,
  MEMCACHED_E2BIG = 37,
  MEMCACHED_INVALID_ARGUMENTS = 38,
  MEMCACHED_KEY_TOO_BIG = 39,
  MEMCACHED_AUTH_PROBLEM = 40,
  MEMCACHED_AUTH_FAILURE = 41,
  MEMCACHED_AUTH_CONTINUE = 42,
  // leaving room for expansion to future memcached error codes
  // the rest of them are numbers we generate
*/
#define MEMCACHED_ARGUMENT_NEEDED      127
#define MEMCACHED_KEY_TOO_LONG         126
#define MEMCACHED_VALUE_TOO_LONG       125
#define MEMCACHED_BAD_INCREMENT        124
#define MEMCACHED_BINARY_PROTO_NEEDED  123

static void mcd_set_operation_result(struct ast_channel *chan, int result) {
	char *numresult = NULL;
	ast_asprintf(&numresult, "%d", result);
	pbx_builtin_setvar_helper(chan, "MCDRESULT", numresult);
}

static int mcd_load_config(void) {

	// initialize the timeout that we wait for a memcached pool operation to complete
	to.tv_sec = 0; to.tv_nsec = 500000;

	struct ast_config *cfg;
	struct ast_flags config_flags = { 0 };

	if (!(cfg = ast_config_load(CONFIG_FILE_NAME, config_flags))) {
		ast_log(LOG_ERROR, "missing memcached resource config file '%s'\n", CONFIG_FILE_NAME);
		return 1;
	} else if (cfg == CONFIG_STATUS_FILEINVALID) {
		ast_log(LOG_ERROR, "memcached resource config file '" CONFIG_FILE_NAME "' invalid format.\n");
		return 1;
	}

    // parse server names for memcached from the [general] section of the config file
    char mcd_config[2048]; mcd_config[0] = 0;
	struct ast_variable *serverentry = ast_variable_browse(cfg, "general");
	for ( ; serverentry; serverentry = serverentry->next) {
		if (strcasecmp(serverentry->name, "server") == 0) {
	    	strcat(mcd_config, "--SERVER=");
    		strcat(mcd_config, serverentry->value);
    		strcat(mcd_config, " ");
		}
	}
    if (strstr(mcd_config, "--SERVER=") == 0) {
        ast_log(LOG_DEBUG, "Expecting memcache server on 127.0.0.1\n");
        strcpy(mcd_config, "--SERVER=127.0.0.1 ");
    }
    ast_log(LOG_DEBUG, "res_memcached configured servers: '%s'\n", mcd_config);
//	strcat(mcd_config, "--SORT-HOSTS ");  not a good idea: turns out that the documentation says:
//                                        "Enabling this will cause hosts that are added to be placed 
//                                         in the host list in sorted order. This will defeat 
//                                         consisten hashing."

	mcdttl = 0;
	const char *ttlvalue;
	if ((ttlvalue = ast_variable_retrieve(cfg, "general", "ttl")))
		mcdttl = atoi(ttlvalue);
	ast_log(LOG_DEBUG, "default time to live for key-value entries set to %d seconds\n", mcdttl);

	use_binary_proto = 1;
	const char *proto_mode;
	if ((proto_mode = ast_variable_retrieve(cfg, "general", "binary_proto")))
		use_binary_proto = ast_true(proto_mode);
//	if (use_binary_proto)
//		strcat(mcd_config, "--BINARY-PROTOCOL ");
//	else
//		ast_log(LOG_WARNING, "not using memcached binary protocol; MCDCOUNTER() function will be unavailable\n");
/*
	const char *hashmode;
	if ((hashmode = ast_variable_retrieve(cfg, "general", "hash"))) {
		strcat(mcd_config, "--HASH=");
		strcat(mcd_config, hashmode);
		strcat(mcd_config, " ");
	}
*/
	const char *kp;
	if ((kp = ast_variable_retrieve(cfg, "general", "keyprefix"))) {
		strcat(mcd_config, "--NAMESPACE=");
		strcat(mcd_config, kp);
		strcat(mcd_config, " ");
	}

    // launch memcached client (pool of)
    mcd_config[strlen(mcd_config) - 1] = 0;
    if ((mcdpool = memcached_pool(mcd_config, strlen(mcd_config))))
	    ast_log(LOG_DEBUG, "res_memcached starting with config: '%s'\n", mcd_config);
	else
	    ast_log(LOG_ERROR, "res_memcached failed to start with config: '%s'\n", mcd_config);

	ast_config_destroy(cfg);
	return 0;

}

static int mcd_read(struct ast_channel *chan, 
	const char *cmd, char *parse, char *buffer, size_t buflen
) {
// asterisk dialplan function that returns the contents of a memcached key

	memcached_return_t rc;
	memcached_st *mcd = memcached_pool_fetch(mcdpool, &to, &rc);
	if (rc) {
        ast_log(LOG_WARNING, "mcd_read: memcached pool error: %d\n", rc);
		return 0;
    }

	char *key = (char *)ast_malloc(MEMCACHED_MAX_KEY);
	buffer[0] = 0;

	if (ast_strlen_zero(parse)) {
		ast_log(LOG_WARNING, "MCD requires argument (key)\n");
		mcd_set_operation_result(chan, MEMCACHED_ARGUMENT_NEEDED);
		free(key);
		return 0;
	}
	strcpy(key, parse);

	memcached_return_t mcdret; size_t szmcdval; uint32_t mcdflags;
	char *mcdval = memcached_get(mcd, key, strlen(key), &szmcdval, &mcdflags, &mcdret);
	if (mcdret)
		ast_log(LOG_WARNING, 
			"MCD() error %d: %s\n", mcdret, memcached_strerror(mcd, mcdret)
		);
	mcd_set_operation_result(chan, mcdret);
	if (mcdret == MEMCACHED_SUCCESS) {
		if (szmcdval > MAX_ASTERISK_VARLEN) {
			ast_log(LOG_WARNING, 
				"returned value (%d bytes) longer that what an asterisk variable can accomodate (%d bytes)\n",
				(int)szmcdval, MAX_ASTERISK_VARLEN
			);
			mcd_set_operation_result(chan, MEMCACHED_VALUE_TOO_LONG);
		} else
			ast_copy_string(buffer, mcdval, buflen);
	}
	free(key);
	memcached_pool_release(mcdpool, mcd);
	return 0;

}

static int mcd_write(
	struct ast_channel *chan, const char *cmd, char *parse, const char *value
) {

	memcached_return_t rc;
	memcached_st *mcd = memcached_pool_fetch(mcdpool, &to, &rc);
	if (rc) {
        ast_log(LOG_WARNING, "mcd_write: memcached pool error: %d\n", rc);
		return 0;
    }

	char *key = (char *)ast_malloc(MEMCACHED_MAX_KEY);
	unsigned int timeout = mcdttl; 

	mcd_set_operation_result(chan, MEMCACHED_SUCCESS);

	// the app argument is the key to set
	if (ast_strlen_zero(parse)) {
		ast_log(LOG_WARNING, "MCD() requires argument (key)\n");
		mcd_set_operation_result(chan, MEMCACHED_ARGUMENT_NEEDED);
		free(key);
		return 0;
	}
	strcpy(key, parse);
	ast_log(LOG_DEBUG, "setting value for key: %s=%s\n", key, value);

	const char *ttlval = pbx_builtin_getvar_helper(chan, "MCDTTL");
	if (ttlval) {
		timeout = atoi(ttlval);
		if ((timeout == 0) && (strcmp(ttlval, "0") != 0)) {
			ast_log(LOG_WARNING, "dialplan variable MCDTTL=%s (not numeric), will use time-to-live value in the config file\n", ttlval);
			timeout = mcdttl;
		}
	}
	ast_log(LOG_DEBUG, "timeout: %d\n", timeout);

	memcached_return_t mcdret = MEMCACHED_FAILURE;
	mcdret = memcached_set(mcd, 
		key, strlen(key), value, strlen(value), (time_t)timeout, (uint32_t)0
	);
	if (mcdret)
		ast_log(LOG_WARNING, 
			"memcached_%s() error %d: %s\n", cmd, mcdret, memcached_strerror(mcd, mcdret)
		);

	mcd_set_operation_result(chan, mcdret);
	free(key);
	memcached_pool_release(mcdpool, mcd);
	return 0;

}

static int mcdget_exec(struct ast_channel *chan, const char *data) {

	memcached_return_t rc;
	memcached_st *mcd = memcached_pool_fetch(mcdpool, &to, &rc);
	if (rc) {
        ast_log(LOG_WARNING, "mcdget_exec: memcached pool error: %d\n", rc);
		return 0;
    }

	char *argcopy;
	char *key = (char *)ast_malloc(MEMCACHED_MAX_KEY);

	mcd_set_operation_result(chan, MEMCACHED_SUCCESS);

	// parse the app arguments
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(varname);
		AST_APP_ARG(key);
	);
	if (ast_strlen_zero(data)) {
		ast_log(LOG_WARNING, "app mcdget requires arguments (varname,key)\n");
		mcd_set_operation_result(chan, MEMCACHED_ARGUMENT_NEEDED);
		free(key);
		memcached_pool_release(mcdpool, mcd);
		return 0;
	}
	argcopy = ast_strdupa(data);
	AST_STANDARD_APP_ARGS(args, argcopy);

	if (ast_strlen_zero(args.key)) {
		ast_log(LOG_WARNING, "key needed\n");
		mcd_set_operation_result(chan, MEMCACHED_ARGUMENT_NEEDED);
		free(key);
		memcached_pool_release(mcdpool, mcd);
		return 0;
	}
	strcpy(key, args.key);
	ast_log(LOG_DEBUG, "key: %s\n", key);

	if (ast_strlen_zero(args.varname)) {
		ast_log(LOG_WARNING, "a valid dialplan variable name is needed as first argument\n");
		mcd_set_operation_result(chan, MEMCACHED_ARGUMENT_NEEDED);
		free(key);
		memcached_pool_release(mcdpool, mcd);
		return 0;
	}
	ast_log(LOG_DEBUG, "setting result into variable '%s'\n", args.varname);
	pbx_builtin_setvar_helper(chan, args.varname, "");

	// get data for key
	memcached_return_t mcdret; size_t szmcdval; uint32_t mcdflags;
	char *mcdval = memcached_get(mcd, key, strlen(key), &szmcdval, &mcdflags, &mcdret);
	if (mcdret)
		ast_log(LOG_WARNING, 
			"memcached_get() error %d: %s\n", mcdret, memcached_strerror(mcd, mcdret)
		);
	mcd_set_operation_result(chan, mcdret);
	if (mcdret == MEMCACHED_SUCCESS) {
		if (szmcdval > MAX_ASTERISK_VARLEN) {
			ast_log(LOG_WARNING, 
				"returned value (%d bytes) longer that what an asterisk variable can accomodate (%d bytes)\n",
				(int)szmcdval, MAX_ASTERISK_VARLEN
			);
			mcd_set_operation_result(chan, MEMCACHED_VALUE_TOO_LONG);
		} else
			pbx_builtin_setvar_helper(chan, args.varname, mcdval);
	}
	free(key);
	memcached_pool_release(mcdpool, mcd);
	return 0;
}

static void mcd_putdata(const char *cmd, struct ast_channel *chan, const char *data) {

	memcached_return_t rc;
	memcached_st *mcd = memcached_pool_fetch(mcdpool, &to, &rc);
	if (rc) {
        ast_log(LOG_WARNING, "mcd_putdata: memcached pool error: %d\n", rc);
		return;
    }

	char *argcopy;
	char *key = (char *)ast_malloc(MEMCACHED_MAX_KEY);
	unsigned int timeout = mcdttl; 

	// parse the app arguments
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(key);
		AST_APP_ARG(val);
	);
	if (ast_strlen_zero(data)) {
		ast_log(LOG_WARNING, "app mcd%s requires arguments (key,value)\n", cmd);
		mcd_set_operation_result(chan, MEMCACHED_ARGUMENT_NEEDED);
		free(key);
		memcached_pool_release(mcdpool, mcd);
		return;
	}
	argcopy = ast_strdupa(data);
	AST_STANDARD_APP_ARGS(args, argcopy);

	if (ast_strlen_zero(args.key)) {
		ast_log(LOG_WARNING, "key needed\n");
		mcd_set_operation_result(chan, MEMCACHED_ARGUMENT_NEEDED);
		free(key);
		memcached_pool_release(mcdpool, mcd);
		return;
	}
	strcpy(key, args.key);
	ast_log(LOG_DEBUG, "key: %s\n", key);

	if (!ast_strlen_zero(args.val))
		ast_log(LOG_DEBUG, "value: %s\n", args.val);
	else
		ast_log(LOG_WARNING, "value is set to zero-length\n");

	const char *ttlval = pbx_builtin_getvar_helper(chan, "MCDTTL");
	if (ttlval) {
		timeout = atoi(ttlval);
		if ((timeout == 0) && (strcmp(ttlval, "0") != 0)) {
			ast_log(LOG_WARNING, "dialplan variable MCDTTL=%s (not numeric), will use time-to-live value in the config file\n", ttlval);
			timeout = mcdttl;
		}
	}
	ast_log(LOG_DEBUG, "timeout: %d\n", timeout);

	memcached_return_t mcdret = MEMCACHED_FAILURE;
	if (strcmp(cmd, "set") == 0)
		mcdret = memcached_set(mcd, 
			key, strlen(key), args.val, strlen(args.val), (time_t)timeout, (uint32_t)0
		);
	else if (strcmp(cmd, "add") == 0)
		mcdret = memcached_add(mcd, 
			key, strlen(key), args.val, strlen(args.val), (time_t)timeout, (uint32_t)0
		);
	else if (strcmp(cmd, "replace") == 0)
		mcdret = memcached_replace(mcd, 
			key, strlen(key), args.val, strlen(args.val), (time_t)timeout, (uint32_t)0
		);
	else if (strcmp(cmd, "append") == 0)
		mcdret = memcached_append(mcd, 
			key, strlen(key), args.val, strlen(args.val), (time_t)timeout, (uint32_t)0
		);

	if (mcdret)
		ast_log(LOG_WARNING, 
			"memcached_%s() error %d: %s\n", cmd, mcdret, memcached_strerror(mcd, mcdret)
		);

	mcd_set_operation_result(chan, mcdret);
	free(key);
	memcached_pool_release(mcdpool, mcd);
	return;

}

static int mcdset_exec(struct ast_channel *chan, const char *data) {
	mcd_putdata("set", chan, data);
	return 0;
}

static int mcdadd_exec(struct ast_channel *chan, const char *data) {
	mcd_putdata("add", chan, data);
	return 0;
}

static int mcdreplace_exec(struct ast_channel *chan, const char *data) {
	mcd_putdata("replace", chan, data);
	return 0;
}

static int mcdappend_exec(struct ast_channel *chan, const char *data) {
	mcd_putdata("append", chan, data);
	return 0;
}

static int mcddelete_exec(struct ast_channel *chan, const char *data) {

	memcached_return_t rc;
	memcached_st *mcd = memcached_pool_fetch(mcdpool, &to, &rc);
	if (rc) {
        ast_log(LOG_WARNING, "mcddelete_exec: memcached pool error: %d\n", rc);
		return 0;
    }

	char *argcopy;
	char *key = (char *)ast_malloc(MEMCACHED_MAX_KEY);

	mcd_set_operation_result(chan, MEMCACHED_SUCCESS);

	// parse the app arguments
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(key);
	);
	if (ast_strlen_zero(data)) {
		ast_log(LOG_WARNING, "app mcddelete requires argument (key)\n");
		mcd_set_operation_result(chan, MEMCACHED_ARGUMENT_NEEDED);
		free(key);
		memcached_pool_release(mcdpool, mcd);
		return 0;
	}
	argcopy = ast_strdupa(data);
	AST_STANDARD_APP_ARGS(args, argcopy);
	
	if (ast_strlen_zero(args.key)) {
		ast_log(LOG_WARNING, "key needed\n");
		mcd_set_operation_result(chan, MEMCACHED_ARGUMENT_NEEDED);
		free(key);
		memcached_pool_release(mcdpool, mcd);
		return 0;
	}
	strcpy(key, args.key);
	ast_log(LOG_DEBUG, "key: %s\n", key);

	memcached_return_t mcdret = memcached_delete(mcd, key, strlen(key), (time_t)0);
	if (mcdret)
		ast_log(LOG_WARNING, 
			"memcached_delete() error %d: %s\n", mcdret, memcached_strerror(mcd, mcdret)
		);
	mcd_set_operation_result(chan, mcdret);
	free(key);
	memcached_pool_release(mcdpool, mcd);
	return 0;

}

static int mcdcounter_read(
	struct ast_channel *chan, const char *cmd, char *parse, char *buffer, size_t buflen
) {

	if (use_binary_proto == 0) {
		ast_log(LOG_WARNING, "MCDCOUNTER() only available when binary protocol is selected\n");
		mcd_set_operation_result(chan, MEMCACHED_BINARY_PROTO_NEEDED);
		return 0;
	}

	memcached_return_t rc;
	memcached_st *mcd = memcached_pool_fetch(mcdpool, &to, &rc);
	if (rc) {
        ast_log(LOG_WARNING, "mcdcounter_read: memcached pool error: %d\n", rc);
		return 0;
    }

	char *argcopy;
	char *key = (char *)ast_malloc(MEMCACHED_MAX_KEY);
	int increment = 0; 

	// parse the app arguments
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(key);
		AST_APP_ARG(increment);
	);
	if (ast_strlen_zero(parse)) {
		ast_log(LOG_WARNING, "MCDCOUNTER() requires arguments (key[,increment])\n");
		mcd_set_operation_result(chan, MEMCACHED_ARGUMENT_NEEDED);
		free(key);
		memcached_pool_release(mcdpool, mcd);
		return 0;
	}
	argcopy = ast_strdupa(parse);
	AST_STANDARD_APP_ARGS(args, argcopy);

	if (!ast_strlen_zero(args.key)) {
		ast_log(LOG_WARNING, "key needed\n");
		mcd_set_operation_result(chan, MEMCACHED_ARGUMENT_NEEDED);
		free(key);
		memcached_pool_release(mcdpool, mcd);
		return 0;
	}
	strcpy(key, args.key);
	ast_log(LOG_DEBUG, "key: %s\n", key);

	if (!ast_strlen_zero(args.increment))
		increment = atoi(args.increment);
	ast_log(LOG_DEBUG, "increment %s by %d\n", key, increment);

	uint64_t newval = 0; 
	memcached_return_t mcdret;
	if (increment >= 0)
		mcdret = memcached_increment(mcd, key, strlen(key), increment, &newval);
	else
		mcdret = memcached_decrement(mcd, key, strlen(key), -increment, &newval);
	if (mcdret)
		ast_log(LOG_WARNING, 
			"MCDCOUNTER() error %d: %s\n", mcdret, memcached_strerror(mcd, mcdret)
		);

	mcd_set_operation_result(chan, mcdret);
	if (mcdret == MEMCACHED_SUCCESS) {
		char *newvalstr = NULL;
		ast_asprintf(&newvalstr, "%d", (int)newval);
		ast_copy_string(buffer, newvalstr, buflen);
	}
	free(key);
	memcached_pool_release(mcdpool, mcd);
	return 0;

}

static int mcdcounter_write(
	struct ast_channel *chan, const char *cmd, char *parse, const char *value
) {

	if (use_binary_proto == 0) {
		ast_log(LOG_WARNING, "MCDCOUNTER() only available when binary protocol is selected\n");
		mcd_set_operation_result(chan, MEMCACHED_BINARY_PROTO_NEEDED);
		return 0;
	}

	memcached_return_t rc;
	memcached_st *mcd = memcached_pool_fetch(mcdpool, &to, &rc);
	if (rc) {
        ast_log(LOG_WARNING, "mcdcounter_write: memcached pool error: %d\n", rc);
		return 0;
    }

	char *key = (char *)ast_malloc(MEMCACHED_MAX_KEY);
	unsigned int counter = 0;
	unsigned int timeout = mcdttl; 

	// the app argument is the key to set
	if (ast_strlen_zero(parse)) {
		ast_log(LOG_WARNING, "MCDCOUNTER() requires argument (key)\n");
		mcd_set_operation_result(chan, MEMCACHED_ARGUMENT_NEEDED);
		free(key);
		memcached_pool_release(mcdpool, mcd);
		return 0;
	}
	strcpy(key, parse);
	ast_log(LOG_DEBUG, "setting counter in key: %s\n", key);

	const char *ttlval = pbx_builtin_getvar_helper(chan, "MCDTTL");
	if (ttlval) {
		timeout = atoi(ttlval);
		if ((timeout == 0) && (strcmp(ttlval, "0") != 0)) {
			ast_log(LOG_WARNING, "dialplan variable MCDTTL=%s (not numeric), will use time-to-live value in the config file\n", ttlval);
			timeout = mcdttl;
		}
	}
	ast_log(LOG_DEBUG, "timeout: %d\n", timeout);

	counter = atoi(value);
	if ((counter == 0) && (strcmp(value, "0") != 0))
		ast_log(LOG_WARNING, "initializing value %s not numeric, will force to 0\n", value);
	ast_log(LOG_DEBUG, "counter: %d\n", (unsigned int)counter);

	memcached_return_t mcdret;
	uint64_t valuenow;
	mcdret = memcached_increment_with_initial(mcd, key, strlen(key), 0, counter, (time_t)timeout, &valuenow);
	if (mcdret)
		ast_log(LOG_WARNING, 
			"memcached_increment_with_initial() error %d: %s\n", mcdret, memcached_strerror(mcd, mcdret)
		);
	mcd_set_operation_result(chan, mcdret);
	free(key);
	memcached_pool_release(mcdpool, mcd);
	return 0;

}

static struct ast_custom_function acf_mcd = {
	.name = "MCD",
	.read = mcd_read,
	.write = mcd_write
};

static struct ast_custom_function acf_mcdcounter = {
	.name = "MCDCOUNTER",
	.read = mcdcounter_read,
	.write = mcdcounter_write
};

static int load_module(void) {
	int ret = 0;
	ret = mcd_load_config();
	ret |= ast_custom_function_register(&acf_mcd);
	ret |= ast_register_application_xml(app_mcdget, mcdget_exec);
	ret |= ast_register_application_xml(app_mcdset, mcdset_exec);
	ret |= ast_register_application_xml(app_mcdadd, mcdadd_exec);
	ret |= ast_register_application_xml(app_mcdreplace, mcdreplace_exec);
	ret |= ast_register_application_xml(app_mcdappend, mcdappend_exec);
	ret |= ast_register_application_xml(app_mcddelete, mcddelete_exec);
	ret |= ast_custom_function_register(&acf_mcdcounter);
	return ret;
}

static int unload_module(void) {
	memcached_pool_destroy(mcdpool);
	int ret = 0;
	ret |= ast_custom_function_unregister(&acf_mcd);
	ret |= ast_unregister_application(app_mcdset);
	ret |= ast_unregister_application(app_mcdget);
	ret |= ast_unregister_application(app_mcdadd);
	ret |= ast_unregister_application(app_mcdreplace);
	ret |= ast_unregister_application(app_mcdappend);
	ret |= ast_unregister_application(app_mcddelete);
	ret |= ast_custom_function_unregister(&acf_mcdcounter);
	return ret;
}

AST_MODULE_INFO_STANDARD(ASTERISK_GPL_KEY, "memcache access functions");
