#include <string.h>
#include "postgres.h"
#include "access/skey.h"
#include "utils/elog.h"
#include "utils/palloc.h"
#include "utils/builtins.h"
#include <time.h>

#define URL_SIZE 1024
#define SIZE_OF_FIELD 256

PG_MODULE_MAGIC;

typedef struct postgres_url_t {
    char raw_url[URL_SIZE]; // https://hello@google.com:80/ULB/SYSTDATA/path?help#3
    char protocol[SIZE_OF_FIELD];   // https://
    char userinfo[SIZE_OF_FIELD];   // hello@
    char host[SIZE_OF_FIELD];       // google.com
    char port[SIZE_OF_FIELD];       // :80
    char path[SIZE_OF_FIELD];       // /ULB/SYSTDATA/path
    char query[SIZE_OF_FIELD];      // ?help
    char fragment[SIZE_OF_FIELD];   // #3
} postgres_url;

postgres_url *postgres_url_from_str(char *str) {
    elog(DEBUG1,"Parsing");
    postgres_url *url = (postgres_url*) palloc(sizeof(postgres_url));

    memset(url->raw_url, 0, SIZE_OF_FIELD);
    memset(url->protocol, 0, SIZE_OF_FIELD);
    memset(url->userinfo, 0, SIZE_OF_FIELD);
    memset(url->host, 0, SIZE_OF_FIELD);
    memset(url->port, 0, SIZE_OF_FIELD);
    memset(url->path, 0, SIZE_OF_FIELD);
    memset(url->query, 0, SIZE_OF_FIELD);
    memset(url->fragment, 0, SIZE_OF_FIELD);

    if (str != NULL) {
        elog(DEBUG1,"Raw URL");
        strcpy(url->raw_url, str);
    }

    bool end_of_parsing = false;

    char *psubstr = NULL;
    char *protocol = NULL;

    elog(DEBUG1,"Protocol");
    if ((psubstr = strstr(str, "://")) != NULL) {
        protocol = palloc(sizeof(char) * (psubstr - str + 1));
        memcpy(protocol, str, psubstr - str);
        protocol[psubstr - str] = '\0';
        psubstr += 3;
        str = psubstr;
    }

    if (protocol != NULL) {
        elog(DEBUG1,"Protocol : %s", protocol);
        strcpy(url->protocol, protocol);
    }

    elog(DEBUG1,"User Info");
    char *userinfo = NULL;
    if ((psubstr = strstr(str, "@")) != NULL) {
        userinfo = palloc(sizeof(char) * (psubstr - str + 1));
        memcpy(userinfo, str, psubstr - str);
        userinfo[psubstr - str] = '\0';
        psubstr += 1;
        str = psubstr;
    }

    if (userinfo != NULL) {
        elog(DEBUG1,"User Info : %s", userinfo);
        strcpy(url->userinfo, userinfo);
    }

    elog(DEBUG1,"Authority");
    char *authority = NULL; // host + port
    if ((psubstr = strstr(str, "/")) != NULL) {
        if (*(psubstr - 1) == '.') {
            psubstr -= 1;
            if (*(psubstr - 1) == '.') {
                psubstr -= 1;
            }
        }
        authority = palloc(sizeof(char) * (psubstr - str + 1));
        memcpy(authority, str, psubstr - str);
        authority[psubstr - str] = '\0';
        str = psubstr;
    } else {
        authority = palloc(sizeof(char) * (strlen(str) + 1));
        strcpy(authority, str);
        end_of_parsing = true;
    }

    if (authority != NULL) {
        elog(DEBUG1,"Authority : %s", authority);
    }

    elog(DEBUG1,"Host and Port");
    char *substr = NULL;
    char *port = NULL;
    char *host = NULL;
    if ((substr = strstr(authority, ":")) != NULL) {
        port = palloc(sizeof(char) * strlen(substr));
        host = palloc(sizeof(char) * (substr - authority + 1));
        memcpy(host, authority, strlen(authority) - strlen(substr));
        host[substr - authority] = '\0';
        strcpy(port, substr);
    } else {
        host = palloc(sizeof(char) * (strlen(authority) + 1));
        memcpy(host, authority, sizeof(char) * strlen(authority));
        host[strlen(authority)] = '\0';
    }

    if (host != NULL) {
        elog(DEBUG1,"Host : %s", host);
        memcpy(url->host, host, strlen(host));
        url->host[strlen(host)] = '\0';
        elog(DEBUG1,"Host : %s", url->host);
    }

    if (port != NULL) {
        elog(DEBUG1,"Port : %s", port);
        memcpy(url->port, port, strlen(port));
    }

    elog(DEBUG1,"Path");
    char *path = NULL;
    if ((psubstr = strstr(str, "?")) != NULL) {
        path = palloc(sizeof(char) * (psubstr - str + 1));
        memcpy(path, str, psubstr - str);
        path[psubstr - str] = '\0';
        str = psubstr;
    } else if ((psubstr = strstr(str, "#")) != NULL) {
        path = palloc(sizeof(char) * (psubstr - str + 1));
        memcpy(path, str, psubstr - str);
        path[psubstr - str] = '\0';
        str = psubstr;
    } else if (!end_of_parsing) {
        path = palloc(sizeof(str));
        strcpy(path, str);
        end_of_parsing = true;
    }

    if (path != NULL) {
        elog(DEBUG1,"Path : %s", path);
        strcpy(url->path, path);
    }

    elog(DEBUG1,"Query");
    char *query = NULL;
    if ((psubstr = strstr(str, "#")) != NULL) {
        query = palloc(sizeof(char) * (psubstr - str + 1));
        memcpy(query, str, psubstr - str);
        query[psubstr - str] = '\0';
        str = psubstr;
    } else if (!end_of_parsing) {
        query = palloc(sizeof(str));
        strcpy(query, str);
        end_of_parsing = true;
    }

    if (query != NULL) {
        elog(DEBUG1,"Query : %s", query);
        strcpy(url->query, query);
    }

    elog(DEBUG1,"Fragment");
    char *fragment = NULL;
    if (strlen(str) > 0 && !end_of_parsing) {
        fragment = palloc(sizeof(str));
        strcpy(fragment, str);
    }

    if (fragment != NULL) {
        elog(DEBUG1,"Fragment : %s", fragment);
        strcpy(url->fragment, fragment);
    }

    return url;
}

char* postgres_url_to_str(postgres_url* url) {
    return url->raw_url;
}

Datum url_in(PG_FUNCTION_ARGS);
Datum url_out(PG_FUNCTION_ARGS);


PG_FUNCTION_INFO_V1(url_in);
Datum
url_in(PG_FUNCTION_ARGS)
{
    elog(DEBUG1,"IN");
    char *str = PG_GETARG_CSTRING(0);
    PG_RETURN_POINTER(postgres_url_from_str(str));
}

PG_FUNCTION_INFO_V1(url_out);
Datum
url_out(PG_FUNCTION_ARGS)
{  
    elog(DEBUG1,"OUT");
    postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING(postgres_url_to_str(url));
}

Datum getAuthority(PG_FUNCTION_ARGS);
Datum getDefaultPort(PG_FUNCTION_ARGS);
Datum getFile(PG_FUNCTION_ARGS);
Datum getHost(PG_FUNCTION_ARGS);
Datum getPath(PG_FUNCTION_ARGS);
Datum getPort(PG_FUNCTION_ARGS);
Datum getProtocol(PG_FUNCTION_ARGS);
Datum getQuery(PG_FUNCTION_ARGS);
Datum getUserInfo(PG_FUNCTION_ARGS);
Datum getRef(PG_FUNCTION_ARGS);
Datum equals(PG_FUNCTION_ARGS);
Datum sameFile(PG_FUNCTION_ARGS);
Datum sameHost(PG_FUNCTION_ARGS);
Datum toString(PG_FUNCTION_ARGS);

// URL type constructors

Datum url_raw_constructor(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_raw_constructor);
Datum url_raw_constructor(PG_FUNCTION_ARGS) {
    PG_RETURN_POINTER(postgres_url_from_str(PG_GETARG_CSTRING(0)));
}

Datum url_all_field_constructor(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_all_field_constructor);
Datum url_all_field_constructor(PG_FUNCTION_ARGS) {
    PG_RETURN_POINTER(postgres_url_from_str(strcat(strcat(strcat(strcat(strcat(
            PG_GETARG_CSTRING(0), "://"), PG_GETARG_CSTRING(1)), ":"), PG_GETARG_CSTRING(2)), PG_GETARG_CSTRING(3))));
}

Datum url_some_field_constructor(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_some_field_constructor);
Datum url_some_field_constructor(PG_FUNCTION_ARGS) {
    PG_RETURN_POINTER(postgres_url_from_str(
            strcat(strcat(strcat(PG_GETARG_CSTRING(0), "://"), PG_GETARG_CSTRING(1)), PG_GETARG_CSTRING(2))));
}

Datum url_copy_constructor(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_copy_constructor);
Datum url_copy_constructor(PG_FUNCTION_ARGS) {
    postgres_url *context = (postgres_url*) PG_GETARG_POINTER(0);
    postgres_url *spec = postgres_url_from_str(PG_GETARG_CSTRING(1));
    // 'https://google.com/drive/u0', 'https://hello@google.com:80../ULB/SYSTDATA/path?help#3

    postgres_url *url = (postgres_url*) palloc(sizeof(postgres_url));

    if (strlen(spec->path) == 0
            && strlen(spec->protocol) == 0
            && strlen(spec->host) == 0
            && strlen(spec->port) == 0
            && strlen(spec->query) == 0) {
        elog(DEBUG1,"Spec not sufficiently defined, took context as url");
        url = context;
    } else {
        elog(DEBUG1,"Spec sufficiently defined, took spec query and fragment");
        if (strlen(spec->query) != 0) strcpy(url->query, spec->query);
        if (strlen(spec->fragment) != 0) strcpy(url->fragment, spec->fragment);
    }

    if (strcmp(context->protocol, spec->protocol) == 0) {
        elog(DEBUG1,"Protocols match, url as protocol : %s", context->protocol);
        strcpy(url->protocol, context->protocol);
    } else {
        elog(DEBUG1,"Protocols mismatch, took spec as url");
        url = spec;
    }

    // TODO : userinfo in authority?
    if (strlen(spec->host) != 0) {
        elog(DEBUG1,"Spec host defined -> used for url host : %s", spec->host);
        strcpy(url->host, spec->host);
        if (strlen(spec->port) != 0) strcpy(url->port, spec->port);
    } else {
        elog(DEBUG1,"No spec host, took context host : %s", context->host);
        strcpy(url->host, context->host);
        if (strlen(context->port) != 0) strcpy(url->port, context->port);
    }

    if (strlen(spec->path) != 0) {
        elog(DEBUG1,"Spec path defined");
        if (*spec->path == '/') {
            elog(DEBUG1,"No relative path, no need to simplify");
            strcpy(url->path, spec->path);
        } else {
            elog(DEBUG1,"Relative path, trying to simplify");
            // compute the nbr of directory to exit from context (file_to_exit)
            char* spec_substr = spec->path;
            int file_to_exit = 0;
            while ((spec_substr = strstr(spec_substr, "/")) != NULL || *(spec_substr + 1) != '.') {
                if (*(spec_substr - 1) != '.' && *(spec_substr - 2) != '.') {
                    file_to_exit++;
                }
            }

            char* path_cpy = palloc(sizeof(char) * strlen(context->path));
            strcpy(path_cpy, context->path);

            // check the number of directory it is possible to exit
            char* context_substr = context->path;
            int nbr_of_file = 0;

            // example : "/drive/u0/ulb"
            while ((context_substr = strstr(context_substr, "/")) != NULL) {
                if (strlen(context_substr) > 1) { // avoid taking into account the last '/' of '/drive/ulb/'
                    nbr_of_file++;
                }
            }

            // end of context path
            char* current = path_cpy + strlen(path_cpy) - 1;
            current--; // last character should not be a '/'
            if (nbr_of_file >= file_to_exit) {
                while (file_to_exit > 0) {
                    if (*(current--) == '/') {
                        memset(current, 0, sizeof(char) * strlen(current));
                        file_to_exit--;
                    }
                }
            }

            strcpy(url->path, strcat(path_cpy, spec_substr));
        }
    } else {
        elog(DEBUG1,"Spec path not defined, took context path %s", context->host);
        if (strlen(context->host) != 0) strcpy(url->host, context->host);
    }

    PG_RETURN_POINTER(url);
}

// Getter

// Authority = host + port
PG_FUNCTION_INFO_V1(getAuthority);
Datum getAuthority(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
    char *host_cpy = palloc(sizeof(char) * strlen(url->host));
    strcpy(host_cpy, url->host);
    PG_RETURN_CSTRING(strcat(host_cpy, url->port));
}

PG_FUNCTION_INFO_V1(getDefaultPort);
Datum getDefaultPort(PG_FUNCTION_ARGS) {
  postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
  int32_t port = 0;
  if (!strcmp(url->protocol, "http"))
    port = 80;
  if (!strcmp(url->protocol, "https"))
    port = 443;
  if (!strcmp(url->protocol, "ftp"))
    port = 21;

  PG_RETURN_INT64(port);
}

// file = path + query
PG_FUNCTION_INFO_V1(getFile);
Datum getFile(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
    char *path_cpy = palloc(sizeof(char) * strlen(url->path));
    strcpy(path_cpy, url->path);
    PG_RETURN_CSTRING(strcat(path_cpy, url->query));
}

PG_FUNCTION_INFO_V1(getProtocol);
Datum getProtocol(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING(url->protocol);
}

PG_FUNCTION_INFO_V1(getUserInfo);
Datum getUserInfo(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING(url->userinfo);
}

PG_FUNCTION_INFO_V1(getHost);
Datum getHost(PG_FUNCTION_ARGS) {
  postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
  PG_RETURN_CSTRING(url->host);
}

PG_FUNCTION_INFO_V1(getPort);
Datum getPort(PG_FUNCTION_ARGS) {
  postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
  PG_RETURN_INT64(url->port);
}

PG_FUNCTION_INFO_V1(getPath);
Datum getPath(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING(url->path);
}

PG_FUNCTION_INFO_V1(getQuery);
Datum getQuery(PG_FUNCTION_ARGS) {
  postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
  PG_RETURN_CSTRING(url->query);
}

PG_FUNCTION_INFO_V1(getRef);
Datum getRef(PG_FUNCTION_ARGS) {
  postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
  PG_RETURN_CSTRING(url->fragment);
}

// Comparasion

PG_FUNCTION_INFO_V1(equals);
Datum equals(PG_FUNCTION_ARGS) {
    postgres_url *url1 = (postgres_url*) PG_GETARG_POINTER(0);
    postgres_url *url2 = (postgres_url*) PG_GETARG_POINTER(1);
    bool res = false;

    if (!strcmp(url1->raw_url, url2->raw_url)) {
        res = true;
    }

    PG_RETURN_BOOL(res);
}

PG_FUNCTION_INFO_V1(sameFile);
Datum sameFile(PG_FUNCTION_ARGS) {
  postgres_url *url1 = (postgres_url*) PG_GETARG_POINTER(0);
  postgres_url *url2 = (postgres_url*) PG_GETARG_POINTER(1);

  // TODO
  PG_RETURN_BOOL(!strcmp("1", "0"));
}

PG_FUNCTION_INFO_V1(sameHost);
Datum sameHost(PG_FUNCTION_ARGS) {
  postgres_url *url1 = (postgres_url*) PG_GETARG_POINTER(0);
  postgres_url *url2 = (postgres_url*) PG_GETARG_POINTER(1);
  PG_RETURN_BOOL(!strcmp(url1->host, url2->host));
}

// toString
PG_FUNCTION_INFO_V1(toString);
Datum toString(PG_FUNCTION_ARGS) {
  postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
  PG_RETURN_CSTRING(postgres_url_to_str(url));
}

PG_FUNCTION_INFO_V1(url_eq);
Datum url_eq(PG_FUNCTION_ARGS) {
    postgres_url *url1 = (postgres_url*) PG_GETARG_POINTER(0);
    postgres_url *url2 = (postgres_url*) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(!strcmp(url1->raw_url, url2->raw_url));
}

PG_FUNCTION_INFO_V1(url_ne);
Datum url_ne(PG_FUNCTION_ARGS) {
    postgres_url *url1 = (postgres_url*) PG_GETARG_POINTER(0);
    postgres_url *url2 = (postgres_url*) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(strcmp(url1->raw_url, url2->raw_url));
}

PG_FUNCTION_INFO_V1(url_lt);
Datum url_lt(PG_FUNCTION_ARGS) {
    postgres_url *url1 = (postgres_url*) PG_GETARG_POINTER(0);
    postgres_url *url2 = (postgres_url*) PG_GETARG_POINTER(1);
    if (strcmp(url1->raw_url, url2->raw_url) < 0){
        PG_RETURN_BOOL(1);//true
    } else {
        PG_RETURN_BOOL(0);//false
    }
}

PG_FUNCTION_INFO_V1(url_le);
Datum url_le(PG_FUNCTION_ARGS) {
    postgres_url *url1 = (postgres_url*) PG_GETARG_POINTER(0);
    postgres_url *url2 = (postgres_url*) PG_GETARG_POINTER(1);
    if (strcmp(url1->raw_url, url2->raw_url) <= 0){
        PG_RETURN_BOOL(1);//true
    } else {
        PG_RETURN_BOOL(0);//false
    }
}

PG_FUNCTION_INFO_V1(url_gt); // >
Datum url_gt(PG_FUNCTION_ARGS) {
    // ((postgres_url*) PG_GETARG_POINTER(0))->raw_url;
    postgres_url *url1 = (postgres_url*) PG_GETARG_POINTER(0);
    postgres_url *url2 = (postgres_url*) PG_GETARG_POINTER(1);
    if (strcmp(url1->raw_url, url2->raw_url) > 0){
        PG_RETURN_BOOL(1);//true
    } else {
        PG_RETURN_BOOL(0);//false
    }
}

PG_FUNCTION_INFO_V1(url_ge);
Datum url_ge(PG_FUNCTION_ARGS) {
    postgres_url *url1 = (postgres_url*) PG_GETARG_POINTER(0);
    postgres_url *url2 = (postgres_url*) PG_GETARG_POINTER(1);
    if (strcmp(url1->raw_url, url2->raw_url) >= 0){
        PG_RETURN_BOOL(1);//true
    } else {
        PG_RETURN_BOOL(0);//false
    }
}

PG_FUNCTION_INFO_V1(url_cmp);
Datum url_cmp(PG_FUNCTION_ARGS) {
    postgres_url *url1 = (postgres_url*) PG_GETARG_POINTER(0);
    postgres_url *url2 = (postgres_url*) PG_GETARG_POINTER(1);
    PG_RETURN_INT64(strcmp(url1->raw_url, url2->raw_url));
}

