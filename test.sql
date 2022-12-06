DROP TABLE IF EXISTS url_table;
DROP EXTENSION IF EXISTS url;

CREATE EXTENSION url;

CREATE TABLE url_table(id int, purl url);
INSERT INTO url_table(id, purl) VALUES(1, 'test');
INSERT INTO url_table(id, purl) VALUES(2, 'google.com');
INSERT INTO url_table(id, purl) VALUES(3, 'https://');
INSERT INTO url_table(id, purl) VALUES(4, 'https://google.com');
INSERT INTO url_table(id, purl) VALUES(5, 'https://hello@google.com');
INSERT INTO url_table(id, purl) VALUES(6, 'https://hello@google.com:80');
INSERT INTO url_table(id, purl) VALUES(7, 'https://hello@google.com:80/ULB/SYSTDATA/path');
INSERT INTO url_table(id, purl) VALUES(8, 'https://hello@google.com/ULB/SYSTDATA/path');
INSERT INTO url_table(id, purl) VALUES(8, 'https://hello@google.com/ULB/SYSTDATA/path?help');
INSERT INTO url_table(id, purl) VALUES(9, 'https://hello@google.com:80/ULB/SYSTDATA/path?help');
INSERT INTO url_table(id, purl) VALUES(10, 'https://hello@google.com:80/ULB/SYSTDATA/path#3');
INSERT INTO url_table(id, purl) VALUES(11, 'https://hello@google.com:80/ULB/SYSTDATA/path?help#3');

SELECT purl, getProtocol(purl), getDefaultPort(purl), getUserInfo(purl), getAuthority(purl), getHost(purl), getPort(purl), getFile(purl), getPath(purl), getQuery(purl), getRef(purl)
FROM url_table;

