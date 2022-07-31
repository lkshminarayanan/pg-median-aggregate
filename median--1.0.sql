CREATE OR REPLACE FUNCTION _median_transfn(state internal, val anyelement)
RETURNS internal
AS 'MODULE_PATHNAME', 'median_transfn'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION _median_finalfn(state internal, val anyelement)
RETURNS anyelement
AS 'MODULE_PATHNAME', 'median_finalfn'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION _median_combinefn(state1 internal, state2 internal)
RETURNS internal
AS 'MODULE_PATHNAME', 'median_combinefn'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION _median_serializefn(state internal)
RETURNS bytea
AS 'MODULE_PATHNAME', 'median_serializefn'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION _median_deserializefn(state bytea, dummy internal)
RETURNS internal
AS 'MODULE_PATHNAME', 'median_deserializefn'
LANGUAGE C IMMUTABLE;

DROP AGGREGATE IF EXISTS median (ANYELEMENT);
CREATE AGGREGATE median (ANYELEMENT)
(
    sfunc = _median_transfn,
    stype = internal,
    finalfunc = _median_finalfn,
    finalfunc_extra,
    combinefunc = _median_combinefn,
    serialfunc = _median_serializefn,
    deserialfunc = _median_deserializefn,
    PARALLEL = SAFE
);
