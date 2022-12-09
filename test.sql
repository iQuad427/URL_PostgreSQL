DROP TABLE IF EXISTS url_table;
DROP EXTENSION IF EXISTS url;

-- CREATE EXTENSION
CREATE EXTENSION url;

-- CREATE TABLE
CREATE TABLE url_table(
    id int,
    p_url url
);

-- INSERTION
INSERT INTO url_table(id, p_url) VALUES(1, 'wikipedia.org');
INSERT INTO url_table(id, p_url) VALUES(2, 'google.com');
INSERT INTO url_table(id, p_url) VALUES(3, 'https://');
INSERT INTO url_table(id, p_url) VALUES(4, '/ULB/SYSTDATA/path');
INSERT INTO url_table(id, p_url) VALUES(5, 'https://google.com');
INSERT INTO url_table(id, p_url) VALUES(6, 'https://hello@google.com');
INSERT INTO url_table(id, p_url) VALUES(7, 'https://hello@google.com:80');
INSERT INTO url_table(id, p_url) VALUES(8, 'https://hello@google.com:80/ULB/SYSTDATA/path');
INSERT INTO url_table(id, p_url) VALUES(9, 'https://hello@google.com/ULB/SYSTDATA/path');
INSERT INTO url_table(id, p_url) VALUES(10, 'https://hello@google.com/ULB/SYSTDATA/path?help');
INSERT INTO url_table(id, p_url) VALUES(11, 'https://hello@google.com:80/ULB/SYSTDATA/path?help');
INSERT INTO url_table(id, p_url) VALUES(12, 'https://hello@google.com:80/ULB/SYSTDATA/path#3');
INSERT INTO url_table(id, p_url) VALUES(13, 'https://hello@google.com:80/ULB/SYSTDATA/path?help#3');
INSERT INTO url_table(id, p_url) VALUES(14, 'https://hello@google.com:80../../../ULB/SYSTDATA/path?help#3');

-- OUTPUT
SELECT * FROM url_table;

-- OPERATORS
SELECT * FROM url_table WHERE p_url < 'https://';
SELECT * FROM url_table WHERE p_url <= 'https://';
SELECT * FROM url_table WHERE p_url = 'https://';
SELECT * FROM url_table WHERE p_url >= 'https://';
SELECT * FROM url_table WHERE p_url > 'https://';
SELECT * FROM url_table WHERE p_url <> 'https://';

-- CONSTRUCTORS

-- Constructor 1
SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path?help#3');
-- Constructor 2
SELECT url('https', 'google.com', 80, '/ceci/est/un/chemin');
-- Constructor 3
SELECT url('https', 'google.com', '/ceci/est/un/chemin');

-- Constructor 4
-- Different protocol -> keep full spec
SELECT url('ftp://me@bonjour/path/file?query#fragmento', 'https://hello@google.com:80/ULB/SYSTDATA/path?help#3');
-- Nothing defined in spec -> keep full context
SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path?help#3', 'hello@');
-- Spec has a defined element -> copy query and fragment from spec
SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path?help#3', '/ceci/est/un/chemin?requete#42');
-- Authority defined in spec -> use spec authority and path
SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path?help#3', 'https://wikipedia.org/path/to/file');
-- Spec path defined and begin with '/' -> use spec path instead
SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path?help#3', '/path/to/file');
-- Spec path is relative (./..) -> appended to context path and simplified (canonical path)
SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path?help#3', '../.././path/to/file');
-- Context path has a '/' to much in path -> not taken into account
SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path/?help#3', '../.././path/to/file');
-- Spec path ask to go out of the context hierarchy -> consider fake relative path and do as if it wasn't there
SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path?help#3', '../../../../.././path/to/file');
-- Relative path is pure noise -> just append spec path to context path
SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path?help#3', '././././././././././path/to/file');

-- INDEXES
DROP INDEX IF EXISTS url_index;
CREATE INDEX url_index ON url_table(p_url);
SET enable_seqscan TO OFF;

EXPLAIN ANALYSE SELECT p_url FROM url_table WHERE equals(p_url, 'https://hello@google.com/ULB/SYSTDATA/path');
EXPLAIN ANALYSE SELECT p_url FROM url_table WHERE p_url < 'https://hello@google.com/ULB/SYSTDATA/path';
EXPLAIN ANALYSE SELECT p_url FROM url_table WHERE p_url <= 'https://hello@google.com/ULB/SYSTDATA/path';
EXPLAIN ANALYSE SELECT p_url FROM url_table WHERE p_url = 'https://hello@google.com/ULB/SYSTDATA/path';
EXPLAIN ANALYSE SELECT p_url FROM url_table WHERE p_url >= 'https://hello@google.com/ULB/SYSTDATA/path';
EXPLAIN ANALYSE SELECT p_url FROM url_table WHERE p_url > 'https://hello@google.com/ULB/SYSTDATA/path';
EXPLAIN ANALYSE SELECT p_url FROM url_table WHERE p_url <> 'https://hello@google.com/ULB/SYSTDATA/path';
EXPLAIN ANALYSE SELECT p_url FROM url_table WHERE getAuthority(p_url) = getAuthority(p_url);
EXPLAIN ANALYSE SELECT p_url FROM url_table WHERE sameHost(p_url, 'https://hello@google.com/ULB/SYSTDATA/path');
EXPLAIN ANALYSE SELECT p_url FROM url_table WHERE sameFile(p_url, 'https://hello@google.com/ULB/SYSTDATA/path');

SET enable_seqscan TO ON;

-- GETTERS
SELECT p_url,
       getProtocol(p_url),
       getDefaultPort(p_url),
       getUserInfo(p_url),
       getAuthority(p_url),
       getHost(p_url),
       getPort(p_url),
       getFile(p_url),
       getPath(p_url),
       getQuery(p_url),
       getRef(p_url)
FROM url_table;

