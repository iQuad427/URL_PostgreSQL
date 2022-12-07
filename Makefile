EXTENSION   = 	url url_gist
MODULES     = 	url
DATA        = 	url--1.0.sql url.control \
				url_gist--1.0.sql url_gist.control

LDFLAGS=-lrt

PG_CONFIG ?= /Applications/Postgres.app/Contents/Versions/14/bin/pg_config
PGXS = $(shell /Applications/Postgres.app/Contents/Versions/14/bin/pg_config --pgxs)
include $(PGXS)
