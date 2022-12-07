-- Comparison
CREATE OR REPLACE FUNCTION equals(url, url)
RETURNS BOOLEAN
AS '$libdir/url', 'equals'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION sameFile(url , url)
RETURNS BOOLEAN
AS '$libdir/url', 'sameFile'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION sameHost(url, url)
RETURNS BOOLEAN
AS '$libdir/url', 'sameHost'
LANGUAGE C IMMUTABLE STRICT;


-- Create the operator class
CREATE OPERATOR CLASS gist_url_ops
    DEFAULT FOR TYPE url USING gist
    AS
    OPERATOR	1	<  ,
    OPERATOR	2	<= ,
    OPERATOR	3	=  ,
    OPERATOR	4	>= ,
    OPERATOR	5	>  ,
    FUNCTION	1	equals(url, url),
    FUNCTION	2	sameHost(url, url),
    FUNCTION	3	sameFile(url, url);
