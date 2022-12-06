EXTENSION   = url
MODULES     = url
DATA        = url--1.0.sql url.control

LDFLAGS=-lrt

PG_CONFIG ?= /Applications/Postgres.app/Contents/Versions/14/bin/pg_config
PGXS = $(shell /Applications/Postgres.app/Contents/Versions/14/bin/pg_config --pgxs)
include $(PGXS)
