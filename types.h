/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2012 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Ruslan Osmanov <osmanov@php.net>                             |
   +----------------------------------------------------------------------+
*/

#ifndef PHP_EV_TYPES_H
#define PHP_EV_TYPES_H

struct ev_watcher;

/* php_ev_object represents Ev* class object. Extends zend_object */

typedef struct php_ev_object {
	zend_object  zo;
	HashTable   *prop_handler;
	void        *ptr; /* Pointer to ev_watcher or php_ev_loop */
} php_ev_object;

/* php_ev_loop pointer is stored in php_ev_object.ptr struct member */

typedef struct php_ev_loop {
	struct ev_loop        *loop;
	zval                  *data;                       /* User custom data attached to event loop                  */
	zend_fcall_info       *fci;                        /* fci and fcc serve callbacks                              */
	zend_fcall_info_cache *fcc;
	double                 io_collect_interval;        /* If > 0, ev_io_collect_interval is called internally      */
	double                 timeout_collect_interval;   /* If > 0, ev_timeout_collect_interval is called internally */
	struct ev_watcher     *w;                          /* Head of linked list                                      */
} php_ev_loop;

/* Property handlers */

typedef int (*php_ev_read_t)(php_ev_object  *obj, zval **retval TSRMLS_DC);
typedef int (*php_ev_write_t)(php_ev_object *obj, zval *newval  TSRMLS_DC);

/* Property of an Ev* class */

typedef struct php_ev_property_entry {
	const char     *name;
	size_t          name_length;
	php_ev_read_t   read_func;
	php_ev_write_t  write_func;
} php_ev_property_entry;

typedef struct {
	char           *name;
	size_t          name_len;
	php_ev_read_t   read_func;
	php_ev_write_t  write_func;
} php_ev_prop_handler;

#endif /* PHP_EV_TYPES_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */