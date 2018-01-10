#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static char *ngx_http_savuri(ngx_conf_t *, void *, void *);
static ngx_conf_post_handler_pt ngx_http_savuri_p = ngx_http_savuri;
/*
 * The structure will hold the value of the
 * module directive savuri
 */
typedef struct {
    ngx_str_t   filename;  /* the file where uri written to */
} ngx_http_savuri_loc_conf_t;

/* 
 * The function which initializes memory for the module configuration structure       
 */
static void * ngx_http_savuri_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_savuri_loc_conf_t  *conf;
    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_savuri_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }
    return conf;
}

/*
 * The command array or array, which holds one subarray for each module
 * directive along with a function which validates the value of the
 * directive and also initializes the main handler of this module
 */
static ngx_command_t ngx_http_savuri_commands[] = {
    { 
        ngx_string("savuri"), 
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1, 
        ngx_conf_set_str_slot, 
        NGX_HTTP_LOC_CONF_OFFSET, 
        offsetof(ngx_http_savuri_loc_conf_t, filename), 
        &ngx_http_savuri_p 
    },
    ngx_null_command
};
static ngx_str_t file_str;

/*
 * The module context has hooks , here we have a hook for creating
 * location configuration
 */
static ngx_http_module_t ngx_http_savuri_module_ctx = {
    NULL,                          /* preconfiguration */
    NULL,                          /* postconfiguration */
    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */
    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */
    ngx_http_savuri_create_loc_conf, /* create location configuration */
    NULL                           /* merge location configuration */
};
/*
 * The module which binds the context and commands
 *
 */
ngx_module_t ngx_http_savuri_module = { 
    NGX_MODULE_V1,
    &ngx_http_savuri_module_ctx,    /* module context */ 
    ngx_http_savuri_commands,       /* module directives */ 
    NGX_HTTP_MODULE,               /* module type */ 
    NULL,                          /* init master */ 
    NULL,                          /* init module */ 
    NULL,                          /* init process */ 
    NULL,                          /* init thread */ 
    NULL,                          /* exit thread */ 
    NULL,                          /* exit process */ 
    NULL,                          /* exit master */ 
    NGX_MODULE_V1_PADDING
};
/*
 * Main handler function of the module.
 */
static ngx_int_t ngx_http_savuri_handler(ngx_http_request_t *r)
{
    ngx_int_t    rc;
    ngx_buf_t   *b;
    ngx_chain_t  out;
    /* we response to 'GET' requests only */
    if (!(r->method & NGX_HTTP_GET)) {
        return NGX_HTTP_NOT_ALLOWED;
    }
    ngx_fd_t fd = NGX_INVALID_FILE;
    //if (fd == NGX_INVALID_FILE)
    fd = ngx_open_file(file_str.data, 
                       NGX_FILE_APPEND,
                       NGX_FILE_CREATE_OR_OPEN,
                       NGX_FILE_DEFAULT_ACCESS);

    if (NGX_INVALID_FILE==fd) 
        return NGX_FILE_ERROR;

    /* fd is open when read the conf */
    ngx_write_fd(fd, r->uri.data, r->uri.len);  
    ngx_write_fd(fd, (u_char*)"\r\n", 2);

    /* */
    ngx_close_file(fd);

    /* set the 'Content-type' header */
    ngx_str_set(&r->headers_out.content_type, "text/html");

    /* allocate a buffer for your response body */
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    /* attach this buffer to the buffer chain */
    out.buf = b;
    out.next = NULL;

    /* adjust the pointers of the buffer */
    b->pos = file_str.data;
    b->last = file_str.data + file_str.len;
    /*
    u_char * rsp = (u_char*)"OK";
    b->pos = rsp;
    b->last = rsp+2;
    */
    b->memory = 1;    /* this buffer is in memory */
    b->last_buf = 1;  /* this is the last buffer in the buffer chain */

    /* set the status line */
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = file_str.len;

    /* send the headers of your response */
    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    /* send the buffer chain of your response */
    return ngx_http_output_filter(r, &out);
}

/*
 * Function for the directive savuri , it validates its value
 * and copies it to a static variable to be printed later
 */
static char * ngx_http_savuri(ngx_conf_t *cf, void *post, void *data)
{
    ngx_http_core_loc_conf_t *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_savuri_handler;

    ngx_str_t  *filename = data; // i.e., first field of ngx_http_savuri_loc_conf_t
    if (ngx_strcmp(filename->data, "") == 0) {
        return NGX_CONF_ERROR;
    }

    file_str.data = filename->data;
    file_str.len  = ngx_strlen(file_str.data);
    return NGX_CONF_OK;
}
