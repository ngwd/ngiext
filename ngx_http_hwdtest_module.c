/*
 * Copyright (C) Weida Huang
 * Copyright (C) R2.AI, Inc.
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static char* ngx_http_hwdtest(ngx_conf_t *, ngx_command_t *, void *); 
static ngx_int_t ngx_http_hwdtest_handler(ngx_http_request_t *); 

static ngx_command_t ngx_http_hwdtest_commands[] = {

    { 
        ngx_string("hwdtest"),
        NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|
        NGX_HTTP_LMT_CONF |NGX_CONF_NOARGS,
        ngx_http_hwdtest,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL,
    },

    ngx_null_command
};

static ngx_http_module_t ngx_http_hwdtest_module_ctx = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

ngx_module_t ngx_http_hwdtest_module = {
    NGX_MODULE_V1, 
    &ngx_http_hwdtest_module_ctx,
    ngx_http_hwdtest_commands,
    NGX_HTTP_MODULE,                       /* module type */  
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING,
};

static char* 
ngx_http_hwdtest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
    ngx_http_core_loc_conf_t *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler =  ngx_http_hwdtest_handler;
    return NGX_CONF_OK;
} 

static ngx_int_t ngx_http_hwdtest_handler(ngx_http_request_t *r) 
{
    if (!(r->method & NGX_HTTP_GET)) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    ngx_fd_t fd = NGX_INVALID_FILE;
    fd = ngx_open_file( (u_char*)"/tmp/test.txt", 
                        NGX_FILE_NONBLOCK|NGX_FILE_APPEND, 
                        NGX_FILE_CREATE_OR_OPEN,
                        0);

    if (fd == NGX_INVALID_FILE) {
        return NGX_ERROR;
    } 

    ssize_t n = ngx_write_fd(fd, r->uri.data, r->uri.len);
    if (n == -1) {
        return NGX_ERROR;
    }

    ngx_int_t fc = ngx_close_file(fd);
    return fc;
}

