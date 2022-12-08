#include "postgres.h"
#include "utils/builtins.h"
#include "utils/elog.h"
#include "utils/palloc.h"
#include <string.h>
#include <time.h>

#define URL_SIZE 512
#define SIZE_OF_FIELD 80

PG_MODULE_MAGIC;

typedef struct postgres_url_t {
  char raw_url[URL_SIZE]; // https://hello@google.com:80/ULB/SYSTDATA/path?help#3
  char protocol[SIZE_OF_FIELD]; // https://
  char userinfo[SIZE_OF_FIELD]; // hello@
  char host[SIZE_OF_FIELD];     // google.com
  int port;                     // :80
  char path[SIZE_OF_FIELD];     // /ULB/SYSTDATA/path
  char query[SIZE_OF_FIELD];    // ?help
  char fragment[SIZE_OF_FIELD]; // #3
} postgres_url;

postgres_url *postgres_url_from_str(char *str) {
  postgres_url *url = (postgres_url *)palloc(sizeof(postgres_url));
  memset(url, 0, sizeof(postgres_url));

  if (str != NULL) {
    strcpy(url->raw_url, str);
  } else {
    ereport(ERROR, (errmsg("Url provided must not be empty")));
  }

  bool end_of_parsing = false;

  char *psubstr = NULL;
  char *protocol = NULL;

  // Protocol
  if ((psubstr = strstr(str, "://")) != NULL) {
    protocol = palloc(sizeof(char) * (psubstr - str + 1));
    memcpy(protocol, str, psubstr - str);
    protocol[psubstr - str] = '\0';
    psubstr += 3;
    str = psubstr;
  }

  if (protocol != NULL) {
    strcpy(url->protocol, protocol);
    pfree(protocol);
  }

  // User Info
  char *userinfo = NULL;
  if ((psubstr = strstr(str, "@")) != NULL) {
    userinfo = palloc(sizeof(char) * (psubstr - str + 1));
    memcpy(userinfo, str, psubstr - str);
    userinfo[psubstr - str] = '\0';
    psubstr += 1;
    str = psubstr;
  }

  if (userinfo != NULL) {
    strcpy(url->userinfo, userinfo);
      pfree(userinfo);
  }

  // Authority
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

  // Host and Port
  char *substr = NULL;
  char *port = NULL;
  char *host = NULL;
  if ((substr = strstr(authority, ":")) != NULL) {
    port = palloc(sizeof(char) * strlen(substr));
    host = palloc(sizeof(char) * (substr - authority + 1));
    memcpy(host, authority, strlen(authority) - strlen(substr));
    host[substr - authority] = '\0';
    strcpy(port, substr + 1);
  } else {
    host = palloc(sizeof(char) * (strlen(authority) + 1));
    memcpy(host, authority, sizeof(char) * strlen(authority));
    host[strlen(authority)] = '\0';
  }

  if (strlen(host) != 0) {
    memcpy(url->host, host, strlen(host));
    url->host[strlen(host)] = '\0';
    pfree(host);
  }

  if (port != NULL) {
    int port_number = atoi(port);
    url->port = port_number;
    pfree(port);
  }

  // Path
  char *path = NULL;
  bool is_query = false;
  if ((psubstr = strstr(str, "?")) != NULL) {
    path = palloc(sizeof(char) * (psubstr - str + 1));
    memcpy(path, str, psubstr - str);
    path[psubstr - str] = '\0';
    str = psubstr;
    is_query = true;
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
    strcpy(url->path, path);
    pfree(path);
  }

  // Query
  char *query = NULL;
  if ((psubstr = strstr(str, "#")) != NULL && is_query) {
    str++;
    query = palloc(sizeof(char) * (psubstr - str + 1));
    memcpy(query, str, psubstr - str);
    query[psubstr - str] = '\0';
    str = psubstr;
  } else if (!end_of_parsing && is_query) {
    str++;
    query = palloc(sizeof(str));
    strcpy(query, str);
    end_of_parsing = true;
  }

  if (query != NULL) {
    strcpy(url->query, query);
    pfree(query);
  }

  // Fragment
  char *fragment = NULL;
  if (strlen(str) > 0 && !end_of_parsing) {
    fragment = palloc(sizeof(str));
    strcpy(fragment, str + 1);
  }

  if (fragment != NULL) {
    strcpy(url->fragment, fragment);
    pfree(fragment);
  }

  return url;
}

char *postgres_url_to_str(postgres_url *url) { return url->raw_url; }

Datum url_in(PG_FUNCTION_ARGS);
Datum url_out(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_in);
Datum url_in(PG_FUNCTION_ARGS) {
  char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(postgres_url_from_str(str));
}

PG_FUNCTION_INFO_V1(url_out);
Datum url_out(PG_FUNCTION_ARGS) {
  postgres_url *url = (postgres_url *)PG_GETARG_POINTER(0);
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
  PG_RETURN_POINTER(postgres_url_from_str(
      strcat(strcat(strcat(strcat(strcat(PG_GETARG_CSTRING(0), "://"),
                                  PG_GETARG_CSTRING(1)),
                           ":"),
                    psprintf("%d", PG_GETARG_INT32(2))),
             PG_GETARG_CSTRING(3))));
}

Datum url_some_field_constructor(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_some_field_constructor);
Datum url_some_field_constructor(PG_FUNCTION_ARGS) {
  PG_RETURN_POINTER(postgres_url_from_str(
      strcat(strcat(strcat(PG_GETARG_CSTRING(0), "://"), PG_GETARG_CSTRING(1)),
             PG_GETARG_CSTRING(2))));
}

char *raw_url_custom_field_constructor(postgres_url *url) {
  char *new_url = palloc(sizeof(char) * URL_SIZE);
  memset(new_url, 0, sizeof(char) * URL_SIZE);

  if (strlen(url->protocol) != 0) {
    strcat(new_url, psprintf("%s://", url->protocol));
  }
  if (strlen(url->userinfo) != 0) {
    strcat(new_url, psprintf("%s@", url->userinfo));
  }
  if (strlen(url->host) != 0) {
    strcat(new_url, url->host);
  }
  if (url->port != 0) {
    strcat(new_url, psprintf(":%d", url->port));
  }
  if (strlen(url->path) != 0) {
    strcat(new_url, url->path);
  }
  if (strlen(url->query) != 0) {
    strcat(new_url, psprintf("?%s", url->query));
  }
  if (strlen(url->fragment) != 0) {
    strcat(new_url, psprintf("#%s", url->fragment));
  }

  return new_url;
}

Datum url_copy_constructor(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_copy_constructor);
Datum url_copy_constructor(PG_FUNCTION_ARGS) {
  postgres_url *context = (postgres_url *)PG_GETARG_POINTER(0);
  postgres_url *spec = postgres_url_from_str(PG_GETARG_CSTRING(1));
  // 'https://google.com/drive/u0',
  // 'https://hello@google.com:80../ULB/SYSTDATA/path?help#3

  postgres_url *url = (postgres_url *)palloc(sizeof(postgres_url));
  memset(url, 0, sizeof(postgres_url));

  if (strlen(spec->path) == 0 && strlen(spec->protocol) == 0 &&
      strlen(spec->host) == 0 && spec->port == 0 && strlen(spec->query) == 0) {
    url = context;

    PG_RETURN_POINTER(url);
  } else {
    if (strlen(spec->query) != 0)
      strcpy(url->query, spec->query);
    if (strlen(spec->fragment) != 0)
      strcpy(url->fragment, spec->fragment);
  }

  if (strlen(spec->protocol) != 0) {
    if (strcmp(context->protocol, spec->protocol) == 0) {
      strcpy(url->protocol, context->protocol);
    } else {
      url = spec;

      PG_RETURN_POINTER(url);
    }
  } else {
    if (strlen(context->protocol) != 0)
      strcpy(url->protocol, context->protocol);
  }

  // TODO : userinfo in authority?
  if (strlen(spec->host) != 0) {
    strcpy(url->host, spec->host);
    if (spec->port != 0)
      url->port = spec->port;
  } else {
    strcpy(url->host, context->host);
    if (context->port != 0)
      url->port = context->port;
  }

  if (strlen(spec->path) != 0) {
    if (*(spec->path) == '/') {
      strcpy(url->path, spec->path);
    } else {
      // compute the nbr of directory to exit from context (file_to_exit)
      char *spec_substr = spec->path;
      int file_to_exit = 0;
      bool forced_end = false;
      int safe_count = 0;
      while ((spec_substr = strstr(spec_substr, "/")) != NULL &&
             *(spec_substr + 1) == '.' && !forced_end) {
        if (*(spec_substr - 1) == '.' && *(spec_substr - 2) == '.') {
          file_to_exit++;
        }
        spec_substr++;

        safe_count++;
        if (safe_count > 1000) {
          forced_end = true;
        }
      }

      char *path_cpy = palloc(sizeof(char) * strlen(context->path));
      strcpy(path_cpy, context->path);

      // check the number of directory it is possible to exit
      char *context_substr = context->path;
      int nbr_of_file = 0;

      // example : "/drive/u0/ulb" -> 3
      forced_end = false;
      safe_count = 0;
      while ((context_substr = strstr(context_substr, "/")) != NULL &&
             !forced_end) {
        if (strlen(context_substr) >
            1) { // avoid taking into account the last '/' of '/drive/ulb/'
          nbr_of_file++;
        }
        context_substr++;

        safe_count++;
        if (safe_count > 1000) {
          forced_end = true;
        }
      }

      forced_end = false;
      safe_count = 0;
      if (nbr_of_file >= file_to_exit) {
        // end of context path
        char *current = path_cpy + strlen(path_cpy) - 1;
        if (*current == '/') { // last character should not be a '/'
          memset(current, 0, 1);
          current--;
        }
        while (file_to_exit > 0 && !forced_end) {
          if (*current == '/') {
            memset(current, 0, sizeof(char) * strlen(current));
            file_to_exit--;
          }
          current--;

          safe_count++;
          if (safe_count > 1000) {
            forced_end = true;
          }
        }
      }

      strcpy(url->path, strcat(path_cpy, spec_substr));
    }
  } else {
    if (strlen(context->host) != 0)
      strcpy(url->host, context->host);
  }

  strcpy(url->raw_url, raw_url_custom_field_constructor(url));

  PG_RETURN_POINTER(url);
}

// Getter

// Authority = host + port
PG_FUNCTION_INFO_V1(getAuthority);
Datum getAuthority(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url *)PG_GETARG_POINTER(0);

    char *user = palloc(sizeof(char) * strlen(url->host));
    memset(user, 0, sizeof(char) * strlen(url->host));
    char *port = palloc(sizeof(char) * 8);
    memset(port, 0, sizeof(char) * 8);

    if (strlen(url->userinfo) != 0) {
        strcpy(user, psprintf("%s@", url->userinfo));
    }
    if (url->port != 0) {
        strcpy(port, psprintf(":%d", url->port));
    }

    PG_RETURN_CSTRING(psprintf("%s%s%s", user, url->host, port));
}

PG_FUNCTION_INFO_V1(getDefaultPort);
Datum getDefaultPort(PG_FUNCTION_ARGS) {
  postgres_url *url = (postgres_url *)PG_GETARG_POINTER(0);
  int32_t port = 0;
  if (!strcmp(url->protocol, "http"))
    port = 80;
  else if (!strcmp(url->protocol, "https"))
    port = 443;
  else if (!strcmp(url->protocol, "ftp"))
    port = 21;

  PG_RETURN_INT64(port);
}

// file = path + query
PG_FUNCTION_INFO_V1(getFile);
Datum getFile(PG_FUNCTION_ARGS) {
  postgres_url *url = (postgres_url *)PG_GETARG_POINTER(0);
  char *path_cpy = palloc(sizeof(char) * strlen(url->path));
  strcpy(path_cpy, url->path);

  char *query = palloc(sizeof(char) * strlen(query) + 1);
  memset(query, 0, sizeof(char) * strlen(query) + 1);
  if (strlen(url->query) != 0) {
    memset(query, 0, sizeof(char) * strlen(query) + 1);
    strcpy(query, psprintf("?%s", url->query));
  }

  PG_RETURN_CSTRING(strcat(path_cpy, query));
}

PG_FUNCTION_INFO_V1(getProtocol);
Datum getProtocol(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url *)PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING(url->protocol);
}

PG_FUNCTION_INFO_V1(getUserInfo);
Datum getUserInfo(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url *)PG_GETARG_POINTER(0);
  PG_RETURN_CSTRING(url->userinfo);
}

PG_FUNCTION_INFO_V1(getHost);
Datum getHost(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url *)PG_GETARG_POINTER(0);
  PG_RETURN_CSTRING(url->host);
}

PG_FUNCTION_INFO_V1(getPort);
Datum getPort(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url *)PG_GETARG_POINTER(0);
  PG_RETURN_INT32(url->port);
}

PG_FUNCTION_INFO_V1(getPath);
Datum getPath(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url *)PG_GETARG_POINTER(0);
  PG_RETURN_CSTRING(url->path);
}

PG_FUNCTION_INFO_V1(getQuery);
Datum getQuery(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url *)PG_GETARG_POINTER(0);
  PG_RETURN_CSTRING(url->query);
}

PG_FUNCTION_INFO_V1(getRef);
Datum getRef(PG_FUNCTION_ARGS) {
    postgres_url *url = (postgres_url *)PG_GETARG_POINTER(0);
  PG_RETURN_CSTRING(url->fragment);
}

// Comparaison

PG_FUNCTION_INFO_V1(equals);
Datum equals(PG_FUNCTION_ARGS) {
  postgres_url *url1 = (postgres_url *)PG_GETARG_POINTER(0);
  postgres_url *url2 = (postgres_url *)PG_GETARG_POINTER(1);
  bool res = false;

  if (!strcmp(url1->raw_url, url2->raw_url)) {
    res = true;
  }

  PG_RETURN_BOOL(res);
}

PG_FUNCTION_INFO_V1(sameFile);
Datum sameFile(PG_FUNCTION_ARGS) {
  postgres_url *url1 = (postgres_url *)PG_GETARG_POINTER(0);
  postgres_url *url2 = (postgres_url *)PG_GETARG_POINTER(1);
  PG_RETURN_BOOL(!strcmp(url1->protocol, url2->protocol) &&
                 !strcmp(url1->userinfo, url2->userinfo) &&
                 !strcmp(url1->host, url2->host) && url1->port == url2->port &&
                 !strcmp(url1->path, url2->path) &&
                 !strcmp(url1->query, url2->query));
}

PG_FUNCTION_INFO_V1(sameHost);
Datum sameHost(PG_FUNCTION_ARGS) {
  postgres_url *url1 = (postgres_url *)PG_GETARG_POINTER(0);
  postgres_url *url2 = (postgres_url *)PG_GETARG_POINTER(1);
  PG_RETURN_BOOL(!strcmp(url1->host, url2->host));
}

// toString
PG_FUNCTION_INFO_V1(toString);
Datum toString(PG_FUNCTION_ARGS) {
  postgres_url *url = (postgres_url *)PG_GETARG_POINTER(0);
  PG_RETURN_CSTRING(postgres_url_to_str(url));
}

PG_FUNCTION_INFO_V1(url_eq);
Datum url_eq(PG_FUNCTION_ARGS) {
  postgres_url *url1 = (postgres_url *)PG_GETARG_POINTER(0);
  postgres_url *url2 = (postgres_url *)PG_GETARG_POINTER(1);
  PG_RETURN_BOOL(!strcmp(url1->raw_url, url2->raw_url));
}

PG_FUNCTION_INFO_V1(url_ne);
Datum url_ne(PG_FUNCTION_ARGS) {
  postgres_url *url1 = (postgres_url *)PG_GETARG_POINTER(0);
  postgres_url *url2 = (postgres_url *)PG_GETARG_POINTER(1);
  PG_RETURN_BOOL(strcmp(url1->raw_url, url2->raw_url));
}

PG_FUNCTION_INFO_V1(url_lt);
Datum url_lt(PG_FUNCTION_ARGS) {
  postgres_url *url1 = (postgres_url *)PG_GETARG_POINTER(0);
  postgres_url *url2 = (postgres_url *)PG_GETARG_POINTER(1);
  if (strcmp(url1->raw_url, url2->raw_url) < 0) {
    PG_RETURN_BOOL(1); // true
  } else {
    PG_RETURN_BOOL(0); // false
  }
}

PG_FUNCTION_INFO_V1(url_le);
Datum url_le(PG_FUNCTION_ARGS) {
  postgres_url *url1 = (postgres_url *)PG_GETARG_POINTER(0);
  postgres_url *url2 = (postgres_url *)PG_GETARG_POINTER(1);
  if (strcmp(url1->raw_url, url2->raw_url) <= 0) {
    PG_RETURN_BOOL(1); // true
  } else {
    PG_RETURN_BOOL(0); // false
  }
}

PG_FUNCTION_INFO_V1(url_gt); // >
Datum url_gt(PG_FUNCTION_ARGS) {
  // ((postgres_url*) PG_GETARG_POINTER(0))->raw_url;
  postgres_url *url1 = (postgres_url *)PG_GETARG_POINTER(0);
  postgres_url *url2 = (postgres_url *)PG_GETARG_POINTER(1);
  if (strcmp(url1->raw_url, url2->raw_url) > 0) {
    PG_RETURN_BOOL(1); // true
  } else {
    PG_RETURN_BOOL(0); // false
  }
}

PG_FUNCTION_INFO_V1(url_ge);
Datum url_ge(PG_FUNCTION_ARGS) {
  postgres_url *url1 = (postgres_url *)PG_GETARG_POINTER(0);
  postgres_url *url2 = (postgres_url *)PG_GETARG_POINTER(1);
  if (strcmp(url1->raw_url, url2->raw_url) >= 0) {
    PG_RETURN_BOOL(1); // true
  } else {
    PG_RETURN_BOOL(0); // false
  }
}

PG_FUNCTION_INFO_V1(url_cmp);
Datum url_cmp(PG_FUNCTION_ARGS) {
  postgres_url *url1 = (postgres_url *)PG_GETARG_POINTER(0);
  postgres_url *url2 = (postgres_url *)PG_GETARG_POINTER(1);
  PG_RETURN_INT64(strcmp(url1->raw_url, url2->raw_url));
}
