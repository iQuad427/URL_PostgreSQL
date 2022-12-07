DROP TABLE IF EXISTS url_table;
DROP EXTENSION IF EXISTS url;

CREATE EXTENSION url;

CREATE TABLE url_table(id int, purl url);
INSERT INTO url_table(id, purl) VALUES(1, 'wikipedia.org');
INSERT INTO url_table(id, purl) VALUES(2, 'google.com');
INSERT INTO url_table(id, purl) VALUES(3, 'https://');
INSERT INTO url_table(id, purl) VALUES(3, '/ULB/SYSTDATA/path');
INSERT INTO url_table(id, purl) VALUES(4, 'https://google.com');
INSERT INTO url_table(id, purl) VALUES(5, 'https://hello@google.com');
INSERT INTO url_table(id, purl) VALUES(6, 'https://hello@google.com:80');
INSERT INTO url_table(id, purl) VALUES(7, 'https://hello@google.com:80/ULB/SYSTDATA/path');
INSERT INTO url_table(id, purl) VALUES(8, 'https://hello@google.com/ULB/SYSTDATA/path');
INSERT INTO url_table(id, purl) VALUES(8, 'https://hello@google.com/ULB/SYSTDATA/path?help');
INSERT INTO url_table(id, purl) VALUES(9, 'https://hello@google.com:80/ULB/SYSTDATA/path?help');
INSERT INTO url_table(id, purl) VALUES(10, 'https://hello@google.com:80/ULB/SYSTDATA/path#3');
INSERT INTO url_table(id, purl) VALUES(11, 'https://hello@google.com:80/ULB/SYSTDATA/path?help#3');
INSERT INTO url_table(id, purl) VALUES(12, 'https://hello@google.com:80../../../ULB/SYSTDATA/path?help#3');


SELECT * FROM url_table
WHERE purl <= 'https://';

-- Constructor 1
SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path?help#3');
-- Constructor 2
SELECT url('https', 'google.com', '80', '/ceci/est/un/chemin');
-- Constructor 3
SELECT url('https', 'google.com', '/ceci/est/un/chemin');

-- Constructor 4
-- Different protocol -> keep full spec
SELECT url('ftp://hello@google.com:80/ULB/SYSTDATA/path?help#3', 'https://hello@google.com:80/ULB/SYSTDATA/path?help#3');
-- Nothing defined in spec -> keep full context
-- SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path?help#3', 'hello@#121');
-- Authority defined in spec -> use spec authority and path
-- SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path?help#3', 'https://wikipedia.org/path/to/file');
-- Spec path defined and begin with '/' -> use spec path instead
-- SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path?help#3', '/path/to/file');
-- Spec path is relative (./..) -> appended to context path and simplified (canonical path)
-- SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path?help#3', '../.././path/to/file');
-- SELECT url('https://hello@google.com:80/ULB/SYSTDATA/path/?help#3', '../.././path/to/file');

SELECT purl, getProtocol(purl), getDefaultPort(purl), getUserInfo(purl), getAuthority(purl), getHost(purl), getPort(purl), getFile(purl), getPath(purl), getQuery(purl), getRef(purl)
FROM url_table;

