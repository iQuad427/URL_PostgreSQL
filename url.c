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
        memcpy(host, authority, substr - authority);
        host[substr - authority] = '\0';
        strcpy(port, substr);
    } else {
        host = palloc(sizeof(char) * (strlen(authority) + 1));
        memcpy(host, authority, sizeof(char) * strlen(authority));
    }

    if (host != NULL) {
        elog(DEBUG1,"Host : %s", host);
        strcpy(url->host, host);
    }

    if (port != NULL) {
        elog(DEBUG1,"Port : %s", port);
        strcpy(url->port, port);
    }

    elog(DEBUG1,"Path");
    char *path = NULL;
    if ((psubstr = strstr(str, "?")) != NULL) {
        elog(DEBUG1,"Saw ? after path");
        elog(DEBUG1,"Read size : %d", (int) (psubstr - str));
        path = palloc(sizeof(char) * (psubstr - str + 1));
        memcpy(path, str, psubstr - str);
        elog(DEBUG1,"Path read : %s", path);
        path[psubstr - str] = '\0';
        str = psubstr;
    } else if ((psubstr = strstr(str, "#")) != NULL) {
        elog(DEBUG1,"Saw # after path");
        elog(DEBUG1,"Read size : %d", (int) (psubstr - str));
        path = palloc(sizeof(char) * (psubstr - str + 1));
        memcpy(path, str, psubstr - str);
        elog(DEBUG1,"Path read : %s", path);
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
  char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(postgres_url_from_str(str));
}

Datum url_all_field_constructor(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_all_field_constructor);
Datum url_all_field_constructor(PG_FUNCTION_ARGS) {
  postgres_url *url = palloc(sizeof(postgres_url));

  char* protocol = PG_GETARG_CSTRING(0);
  char* host = PG_GETARG_CSTRING(1);
  char* port = PG_GETARG_CSTRING(2);
  char* path = PG_GETARG_CSTRING(3);

  strcpy(url->protocol, protocol);
  strcpy(url->host, host);
  strcpy(url->port, port);
  strcpy(url->path, path);

  PG_RETURN_POINTER(url);
}

Datum url_some_field_constructor(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_some_field_constructor);
Datum url_some_field_constructor(PG_FUNCTION_ARGS) {
  postgres_url *url = palloc(sizeof(postgres_url));

  strcpy(url->protocol, PG_GETARG_CSTRING(0));
  strcpy(url->host, PG_GETARG_CSTRING(1));
  strcpy(url->path, PG_GETARG_CSTRING(2));

  PG_RETURN_POINTER(url);
}

Datum url_copy_constructor(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_copy_constructor);
Datum url_copy_constructor(PG_FUNCTION_ARGS) {
  postgres_url *context = palloc(sizeof(postgres_url));
  context = PG_GETARG_POINTER(0);

  postgres_url *spec = palloc(sizeof(postgres_url));
  spec = postgres_url_from_str(PG_GETARG_CSTRING(1));

  postgres_url *url = palloc(sizeof(postgres_url));

  if (strcmp(context->protocol, spec->protocol)) {
    url = spec;
  } else {
    if (spec->userinfo != NULL && spec->host != NULL) {
      strcpy(url->userinfo, spec->userinfo);
      strcpy(url->host, spec->host);
    } else {
      strcpy(url->userinfo, context->userinfo);
      strcpy(url->host, context->host);
    }

    if (!strcmp(spec->path[0], "/")) {
      strcpy(url->path, spec->path);
    } else {
      // TODO : should remove './..' characters from spec path (cf. JavaDoc)
      strcpy(url->path, strcat(context->path, spec->path));
    }
  }

  PG_RETURN_POINTER(url);
}

// Getter

// Authority = host + port
PG_FUNCTION_INFO_V1(getAuthority);
Datum getAuthority(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING(strcat(url->host, url->port));
}

PG_FUNCTION_INFO_V1(getDefaultPort);
Datum getDefaultPort(PG_FUNCTION_ARGS) {
  postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
  int32_t port = 0;
  if (!strcmp(url->protocol, "http://"))
    port = 80;
  if (!strcmp(url->protocol, "https://"))
    port = 443;
  if (!strcmp(url->protocol, "ftp://"))
    port = 21;

  PG_RETURN_INT64(port);
}

// file = path + query
PG_FUNCTION_INFO_V1(getFile);
Datum getFile(PG_FUNCTION_ARGS) {
  postgres_url *url = (postgres_url*) PG_GETARG_POINTER(0);
  PG_RETURN_CSTRING(strcat(url->path, url->query));
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
