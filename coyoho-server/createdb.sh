#!/bin/bash
dropdb -U postgres coyoho
createdb -U postgres -O coyoho coyoho
#psql -U coyoho coyoho < db/db-schema.sql
#psql -U coyoho coyoho < db/db-data.sql
rake migrate
