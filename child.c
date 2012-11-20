
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
#include "watcher.h"

/* {{{ proto EvChild::__construct(int pid, bool trace, EvLoop loop, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvChild, __construct)
{
	long           pid;
	zend_bool      trace;
	zval          *self;
	php_ev_object *o_self;
	php_ev_object *o_loop;
	ev_child      *child_watcher;

	zval                  *loop;
	zval                  *data       = NULL;
	zend_fcall_info        fci        = empty_fcall_info;
	zend_fcall_info_cache  fcc        = empty_fcall_info_cache;
	long                   priority   = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lbOf|z!l",
				&pid, &trace, &loop, ev_loop_class_entry_ptr, &fci, &fcc,
				&data, &priority) == FAILURE) {
		return;
	}

	self          = getThis();
	o_self        = (php_ev_object *) zend_object_store_get_object(self TSRMLS_CC);
	o_loop        = (php_ev_object *) zend_object_store_get_object(loop TSRMLS_CC);
	child_watcher = (ev_child *) php_ev_new_watcher(sizeof(ev_child), self,
			PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(o_loop),
			&fci, &fcc, data, priority TSRMLS_CC);

	child_watcher->type = EV_CHILD;
	
	ev_child_set(child_watcher, pid, trace);

	o_self->ptr = (void *) child_watcher;
}
/* }}} */

/* {{{ proto void EvChild::set(int pid, bool trace) */
PHP_METHOD(EvChild, set)
{
	long       pid;
	zend_bool  trace;
	ev_child  *child_watcher;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "bl",
				&pid, &trace) == FAILURE) {
		return;
	}

	child_watcher = (ev_child *) PHP_EV_WATCHER_FETCH_FROM_THIS();

	PHP_EV_WATCHER_RESET(ev_child, child_watcher, (child_watcher, pid, trace));
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */