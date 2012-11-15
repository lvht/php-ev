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

#include "embed.h"
#include "priv.h"
#include "watcher.h"

/* Defined in ev.c */
extern zend_class_entry *ev_loop_class_entry_ptr;

/* {{{ php_ev_set_watcher_priority() */
inline void php_ev_set_watcher_priority(ev_watcher *watcher, long priority TSRMLS_DC)
{
	PHP_EV_CHECK_PENDING_WATCHER(watcher);	
	ev_set_priority(watcher, priority);
}
/* }}} */

/* {{{ php_ev_watcher_callback() */
void php_ev_watcher_callback(EV_P_ ev_watcher *watcher, int revents)
{
	zval            **args[2];
	zval             *key1;
	zval             *key2;
	zval             *retval_ptr;
	zval             *self       = php_ev_watcher_self(watcher);
	zend_fcall_info  *pfci       = php_ev_watcher_fci(watcher);

	TSRMLS_FETCH_FROM_CTX(php_ev_watcher_thread_ctx(watcher));

	if (revents & EV_ERROR) {
		int errorno = errno;
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Got unspecified libev error in revents, errno = %d, err = %s", errorno, strerror(errorno));

		PHP_EV_EXIT_LOOP(EV_A);
	}
#if EV_STAT_ENABLE
	/* TODO: php_ev_stat_update()*/
	else if (revents & EV_STAT /* && php_ev_stat_update(watcher) */) {
		PHP_EV_EXIT_LOOP(EV_A);
	}
#endif
	else if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback args */
		key1 = self;
		args[0] = &key1;
		zval_add_ref(&key1);

		MAKE_STD_ZVAL(key2);
		args[1] = &key2;
		ZVAL_LONG(key2, revents);

		/* Prepare callback */
		pfci->params         = args;
		pfci->retval_ptr_ptr = &retval_ptr;
		pfci->param_count    = 2;
		pfci->no_separation  = 0;

		if (zend_call_function(pfci, php_ev_watcher_fcc(watcher) TSRMLS_CC) == SUCCESS
		        && retval_ptr) {
		    zval_ptr_dtor(&retval_ptr);
		} else {
		    php_error_docref(NULL TSRMLS_CC, E_WARNING,
		            "An error occurred while invoking the callback");
		}

		zval_ptr_dtor(&key1);
		zval_ptr_dtor(&key2);
	}
}
/* }}} */

/* {{{ php_ev_new_watcher()
 * Create watcher of the specified type, initialize common watcher fields
 */
void *php_ev_new_watcher(size_t size, zval *self, php_ev_loop *loop, const zend_fcall_info *pfci, const zend_fcall_info_cache *pfcc, zval *data, int priority TSRMLS_DC)
{
	void *w = emalloc(size);
	memset(w, 0, size);

	ev_watcher *w_prev = loop->w;
	loop->w = (ev_watcher *) w;	

	if (w_prev) {
		php_ev_watcher_prev(w) = (void *) w_prev;
	}

	ev_init((ev_watcher *) w, (ZEND_FCI_INITIALIZED(*pfci) ? php_ev_watcher_callback : 0));

	Z_ADDREF_P(self);
	if (data) {
		Z_ADDREF_P(data);
	}

	php_ev_watcher_self(w)  = self;
	php_ev_watcher_data(w)  = data;
	php_ev_watcher_loop(w)  = loop;
	php_ev_watcher_flags(w) = PHP_EV_WATCHER_FLAG_KEEP_ALIVE;

	PHP_EV_COPY_FCALL_INFO(php_ev_watcher_fci(w), php_ev_watcher_fcc(w), pfci, pfcc);

	php_ev_set_watcher_priority((ev_watcher *) w, priority TSRMLS_CC);

	TSRMLS_SET_CTX(php_ev_watcher_thread_ctx(w));
	
	return w;
}
/* }}} */

/* {{{ php_ev_start_watcher() */
static void php_ev_start_watcher(ev_watcher *watcher TSRMLS_DC)
{
	switch (watcher->type) {
		case EV_IO:
			PHP_EV_WATCHER_START(ev_io, watcher);
			break;
		case EV_TIMER:
			PHP_EV_WATCHER_START(ev_timer, watcher);
			break;
#if EV_PERIODIC_ENABLE
		case EV_PERIODIC:
			PHP_EV_WATCHER_START(ev_periodic, watcher);
			break;
#endif
#if EV_SIGNAL_ENABLE
		case EV_SIGNAL:
			PHP_EV_WATCHER_START(ev_signal, watcher);
			break;
#endif
#if EV_CHILD_ENABLE
		case EV_CHILD:
			PHP_EV_WATCHER_START(ev_child, watcher);
			break;
#endif
#if EV_STAT_ENABLE
		case EV_STAT:
			PHP_EV_WATCHER_START(ev_stat, watcher);
			break;
#endif
#if EV_IDLE_ENABLE
		case EV_IDLE:
			PHP_EV_WATCHER_START(ev_idle, watcher);
			break;
#endif
#if EV_PREPARE_ENABLE
		case EV_PREPARE:
			PHP_EV_WATCHER_START(ev_prepare, watcher);
			break;
#endif
#if EV_CHECK_ENABLE
		case EV_CHECK:
			PHP_EV_WATCHER_START(ev_check, watcher);
			break;
#endif
#if EV_EMBED_ENABLE
		case EV_EMBED_ENABLE:
			PHP_EV_WATCHER_START(ev_embed, watcher);
			break;
#endif
#if EV_FORK_ENABLE
		case EV_FORK_ENABLE:
			PHP_EV_WATCHER_START(ev_fork, watcher);
			break;
#endif
#if EV_ASYNC_ENABLE
		case EV_ASYNC:
			PHP_EV_WATCHER_START(ev_async, watcher);
			break;
#endif
		default:
			break;
	}
}
/* }}} */

/* {{{ php_ev_stop_watcher() */
void php_ev_stop_watcher(ev_watcher *watcher TSRMLS_DC)
{
	switch (php_ev_watcher_type(watcher)) {
		case EV_IO:
			PHP_EV_WATCHER_STOP(ev_io, watcher);
			break;
		case EV_TIMER:
			PHP_EV_WATCHER_STOP(ev_timer, watcher);
			break;
#if EV_PERIODIC_ENABLE
		case EV_PERIODIC:
			PHP_EV_WATCHER_STOP(ev_periodic, watcher);
			break;
#endif
#if EV_SIGNAL_ENABLE
		case EV_SIGNAL:
			PHP_EV_WATCHER_STOP(ev_signal, watcher);
			break;
#endif
#if EV_CHILD_ENABLE
		case EV_CHILD:
			PHP_EV_WATCHER_STOP(ev_child, watcher);
			break;
#endif
#if EV_STAT_ENABLE
		case EV_STAT:
			PHP_EV_WATCHER_STOP(ev_stat, watcher);
			break;
#endif
#if EV_IDLE_ENABLE
		case EV_IDLE:
			PHP_EV_WATCHER_STOP(ev_idle, watcher);
			break;
#endif
#if EV_PREPARE_ENABLE
		case EV_PREPARE:
			PHP_EV_WATCHER_STOP(ev_prepare, watcher);
			break;
#endif
#if EV_CHECK_ENABLE
		case EV_CHECK:
			PHP_EV_WATCHER_STOP(ev_check, watcher);
			break;
#endif
#if EV_EMBED_ENABLE
		case EV_EMBED_ENABLE:
			PHP_EV_WATCHER_STOP(ev_embed, watcher);
			break;
#endif
#if EV_FORK_ENABLE
		case EV_FORK_ENABLE:
			PHP_EV_WATCHER_STOP(ev_fork, watcher);
			break;
#endif
#if EV_ASYNC_ENABLE
		case EV_ASYNC:
			PHP_EV_WATCHER_STOP(ev_async, watcher);
			break;
#endif
		default:
			break;
	}
}
/* }}} */


/* {{{ Methods */

/* {{{ proto void EvWatcher::start(void) */
PHP_METHOD(EvWatcher, start)
{
	php_ev_object *o_self;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	o_self = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	php_ev_start_watcher(PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self) TSRMLS_CC);
}
/* }}} */

/* {{{ proto void EvWatcher::stop(void) */
PHP_METHOD(EvWatcher, stop)
{
	php_ev_object *o_self;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	o_self = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	php_ev_stop_watcher(PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self) TSRMLS_CC);
}
/* }}} */

/* {{{ proto int EvWatcher::clear(void) */
PHP_METHOD(EvWatcher, clear)
{
	php_ev_object *o_self;
	ev_watcher    *w;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	o_self = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	w      = PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self);

	RETURN_LONG((long)ev_clear_pending(php_ev_watcher_loop_ptr(w), w));
}
/* }}} */

/* {{{ proto void EvWatcher::invoke(int revents) */
PHP_METHOD(EvWatcher, invoke)
{
	php_ev_object *o_self;
	ev_watcher    *w;
	long           revents;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&revents) == FAILURE) {
		return;
	}

	o_self = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	w      = PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self);

	ev_invoke(php_ev_watcher_loop_ptr(w), w, (int)revents);
}
/* }}} */

/* {{{ proto void EvWatcher::feed(int revents) */
PHP_METHOD(EvWatcher, feed)
{
	php_ev_object *o_self;
	ev_watcher    *w;
	long           revents;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&revents) == FAILURE) {
		return;
	}

	o_self = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	w      = PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self);

	ev_feed_event(php_ev_watcher_loop_ptr(w), w, (int)revents);
}
/* }}} */

/* {{{ proto EvLoop EvWatcher::get_loop(void) */
PHP_METHOD(EvWatcher, get_loop)
{
	/*
	 * RETURNING REFERENCES - 
	 * Sara Goleman, ... p.71
	 */

	php_ev_object *o_self;
	php_ev_loop *loop;
	ev_watcher *w;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	o_self = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	w      = PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self);
	loop   = php_ev_watcher_loop(w);

	zval *zloop = (zval *) ev_userdata(loop->loop);

    if (!zloop) {
    	RETURN_NULL();
    }
	RETVAL_ZVAL(zloop, 1, 0);

#if 0
	php_ev_object *o_loop;
	Z_TYPE_P(return_value) = IS_OBJECT;
	object_init_ex(return_value, ev_loop_class_entry_ptr);
	Z_SET_REFCOUNT_P(return_value, 1);
	Z_UNSET_ISREF_P(return_value);

	o_loop = (php_ev_object *) zend_object_store_get_object(return_value TSRMLS_CC);
	o_loop->ptr = (void *) loop;

    if (loop->data) {
		Z_ADDREF_P(loop->data);
    }

	if (loop->fci) {
		if (ZEND_FCI_INITIALIZED(*loop->fci)) {
			PHP_EV_FCI_ADDREF(loop->fci);
		}
	}
#endif
	
#if 0 /* {{{ */
	php_ev_object *o_self;
	ev_watcher    *w;
	zval          *loop;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	o_self = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	w      = PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self);

	loop   = php_ev_watcher_loop(w);

    if (!loop) {
    	RETURN_NULL();
    }
	RETVAL_ZVAL(loop, 1, 0);
#endif /* }}} */

}
/* }}} */

/* {{{ proto int EvWatcher::keepalive(int value = 1) */
PHP_METHOD(EvWatcher, keepalive)
{
	ev_watcher    *w;
	long           n_value = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l",
				&n_value) == FAILURE) {
		return;
	}

	w = PHP_EV_WATCHER_FETCH_FROM_THIS();

	/* Returning previous state */
	RETVAL_LONG((w->e_flags & PHP_EV_WATCHER_FLAG_KEEP_ALIVE));
	n_value = n_value ? PHP_EV_WATCHER_FLAG_KEEP_ALIVE : 0;

	/* Update watcher flags, if keepalive flag changed */
	if ((n_value ^ w->e_flags) & PHP_EV_WATCHER_FLAG_KEEP_ALIVE) {
		w->e_flags = (w->e_flags & ~PHP_EV_WATCHER_FLAG_KEEP_ALIVE) | n_value;
		PHP_EV_WATCHER_REF(w);
		PHP_EV_WATCHER_UNREF(w);
	}
}
/* }}} */

/* {{{ proto void EvWatcher::set_callback(callable callback) */
PHP_METHOD(EvWatcher, set_callback)
{
	ev_watcher            *w;
	zend_fcall_info        fci;
	zend_fcall_info_cache  fcc;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f",
				&fci, &fcc) == FAILURE) {
		return;
	}

	w = PHP_EV_WATCHER_FETCH_FROM_THIS();

	PHP_EV_FREE_FCALL_INFO(w->fci, w->fcc);
	PHP_EV_COPY_FCALL_INFO(w->fci, w->fcc, &fci, &fcc);
}
/* }}} */

/* Methods }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */