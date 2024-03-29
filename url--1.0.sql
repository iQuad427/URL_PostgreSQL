-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION url" to load this file. \quit

CREATE OR REPLACE FUNCTION url_in(cstring)
RETURNS url
AS '$libdir/url'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url_out(url)
RETURNS cstring
AS '$libdir/url'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE url (
	INPUT          = url_in,
	OUTPUT         = url_out,
    INTERNALLENGTH = 1024
);
COMMENT ON TYPE url IS 'url datatype for PostgreSQL';

-- Constructors
CREATE OR REPLACE FUNCTION url(cstring)
RETURNS url
AS '$libdir/url', 'url_raw_constructor'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url(cstring, cstring, int, cstring)
RETURNS url
AS '$libdir/url', 'url_all_field_constructor'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url(cstring, cstring, cstring)
RETURNS url
AS '$libdir/url', 'url_some_field_constructor'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url(url, cstring)
RETURNS url
AS '$libdir/url', 'url_copy_constructor'
LANGUAGE C IMMUTABLE STRICT;

-- Getters
CREATE OR REPLACE FUNCTION getAuthority(url)
RETURNS TEXT
AS '$libdir/url', 'getAuthority'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION getDefaultPort(url)
RETURNS INTEGER
AS '$libdir/url', 'getDefaultPort'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION getFile(url)
RETURNS TEXT
AS '$libdir/url', 'getFile'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION getHost(url)
RETURNS TEXT
AS '$libdir/url', 'getHost'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION getPath(url)
RETURNS TEXT
AS '$libdir/url', 'getPath'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION getPort(url)
RETURNS INTEGER
AS '$libdir/url', 'getPort'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION getProtocol(url)
RETURNS TEXT
AS '$libdir/url', 'getProtocol'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION getQuery(url)
RETURNS TEXT
AS '$libdir/url', 'getQuery'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION getRef(url)
RETURNS TEXT
AS '$libdir/url', 'getRef'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION getUserInfo(url)
RETURNS TEXT
AS '$libdir/url', 'getUserInfo'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION toString(url)
RETURNS cstring
AS '$libdir/url', 'toString'
LANGUAGE C IMMUTABLE STRICT;


-- Index Compatible Comparison

CREATE OR REPLACE FUNCTION equals(url1 url, url2 url)
RETURNS BOOLEAN
AS 'SELECT url1 = url2'
LANGUAGE SQL IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION sameHost(url1 url, url2 url)
RETURNS BOOLEAN
AS 'SELECT getHost(url1) = getHost(url2)'
LANGUAGE SQL IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION sameFile(url1 url, url2 url)
RETURNS BOOLEAN
AS 'SELECT getAuthority(url1) = getAuthority(url2) AND getProtocol(url1) = getProtocol(url2) AND getFile(url1) = getFile(url2)'
LANGUAGE SQL IMMUTABLE STRICT;


-- Comparison

CREATE OR REPLACE FUNCTION url_eq(url, url)
RETURNS BOOLEAN
AS '$libdir/url', 'url_eq'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url_ne(url, url)
RETURNS BOOLEAN
AS '$libdir/url', 'url_ne'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url_lt(url, url)
RETURNS BOOLEAN
AS '$libdir/url', 'url_lt'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url_le(url, url)
RETURNS BOOLEAN
AS '$libdir/url', 'url_le'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url_gt(url, url)
RETURNS BOOLEAN
AS '$libdir/url', 'url_gt'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url_ge(url, url)
RETURNS BOOLEAN
AS '$libdir/url', 'url_ge'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION url_cmp(url, url)
RETURNS INTEGER
AS '$libdir/url', 'url_cmp'
LANGUAGE C IMMUTABLE STRICT;

-- Comparison index

CREATE OPERATOR = (
	LEFTARG = url,
	RIGHTARG = url,
	PROCEDURE = url_eq,
	COMMUTATOR = =,
	NEGATOR = <>,
	RESTRICT = eqsel,
	JOIN = eqjoinsel
);
COMMENT ON OPERATOR =(url, url) IS 'equals?';

CREATE OPERATOR <> (
	LEFTARG = url,
	RIGHTARG = url,
	PROCEDURE = url_ne,
	COMMUTATOR = <>,
	NEGATOR = =,
	RESTRICT = neqsel,
	JOIN = neqjoinsel
);
COMMENT ON OPERATOR <>(url, url) IS 'not equals?';

CREATE OPERATOR < (
	LEFTARG = url,
	RIGHTARG = url,
	PROCEDURE = url_lt,
	COMMUTATOR = > ,
	NEGATOR = >= , 
   	RESTRICT = scalarltsel, 
	JOIN = scalarltjoinsel
);
COMMENT ON OPERATOR <(url, url) IS 'less-than';

CREATE OPERATOR <= (
	LEFTARG = url,
	RIGHTARG = url,
	PROCEDURE = url_le,
	COMMUTATOR = >= , 
	NEGATOR = > ,
   	RESTRICT = scalarltsel, 
	JOIN = scalarltjoinsel
);
COMMENT ON OPERATOR <=(url, url) IS 'less-than-or-equal';

CREATE OPERATOR > (
	LEFTARG = url,
	RIGHTARG = url,
	PROCEDURE = url_gt,
	COMMUTATOR = < ,
	NEGATOR = <= ,
   	RESTRICT = scalargtsel, 
	JOIN = scalargtjoinsel
);
COMMENT ON OPERATOR >(url, url) IS 'greater-than';

CREATE OPERATOR >= (
	LEFTARG = url,
	RIGHTARG = url,
	PROCEDURE = url_ge,
	COMMUTATOR = <= , 
	NEGATOR = < ,
   	RESTRICT = scalargtsel, 
	JOIN = scalargtjoinsel
);
COMMENT ON OPERATOR >=(url, url) IS 'greater-than-or-equal';

CREATE OPERATOR CLASS btree_url_ops
DEFAULT FOR TYPE url USING btree
AS
        OPERATOR        1       <  ,
        OPERATOR        2       <= ,
        OPERATOR        3       =  ,
        OPERATOR        4       >= ,
        OPERATOR        5       >  ,
        FUNCTION        1       url_cmp(url, url);