CREATE TABLE intvals(val int, color text);

-- Test empty table
SELECT median(val) FROM intvals;

-- Integers with odd number of values
INSERT INTO intvals VALUES
       (1, 'a'),
       (2, 'c'),
       (9, 'b'),
       (7, 'c'),
       (2, 'd'),
       (-3, 'd'),
       (2, 'e');

SELECT * FROM intvals ORDER BY val;
SELECT median(val) FROM intvals;

-- Integers with NULLs and even number of values
INSERT INTO intvals VALUES
       (99, 'a'),
       (NULL, 'a'),
       (NULL, 'e'),
       (NULL, 'b'),
       (7, 'c'),
       (0, 'd');

SELECT * FROM intvals ORDER BY val;
SELECT median(val) FROM intvals;

-- Real values
CREATE TABLE realvals(val real, color text);
INSERT INTO realvals VALUES
        (76.2, 'a'),
        (33.71, 'c'),
        (21.5, 'b'),
        (56.38, 'c'),
        (10, 'd'),
        (99.12, 'd'),
        (24.98, 'e'),
        (33.72, 'f');

SELECT * FROM realvals ORDER BY val;
SELECT median(val) FROM realvals;

-- Text values
CREATE TABLE textvals(val text, color int);

INSERT INTO textvals VALUES
       ('erik', 1),
       ('mat', 3),
       ('rob', 8),
       ('david', 9),
       ('lee', 2);

SELECT * FROM textvals ORDER BY val;
SELECT median(val) FROM textvals;

-- Test large table with timestamps
CREATE TABLE timestampvals (val timestamptz);

SET max_parallel_workers_per_gather=4;

INSERT INTO timestampvals(val)
SELECT TIMESTAMP 'epoch' + (i * INTERVAL '1 second')
FROM generate_series(0, 10000000) as T(i);

-- expected median
SELECT percentile_disc(0.5) WITHIN GROUP (ORDER BY val) as expected_median
FROM timestampvals;
-- actual median
SELECT median(val) FROM timestampvals;

-- Test collation handling by text comparators;
CREATE COLLATION numeric (provider = icu, locale = 'en@colNumeric=yes');
CREATE TABLE textvals2(valDefaultCollation text, valNumericOrder text COLLATE numeric);
INSERT INTO textvals2 VALUES
         ('a1', 'a1'),
         ('a2', 'a2'),
         ('a3', 'a3'),
         ('a4', 'a4'),
         ('a5', 'a5'),
         ('a6', 'a6'),
         ('a10', 'a10'),
         ('a20', 'a20'),
         ('a30', 'a30'),
         ('a40', 'a40'),
         ('a50', 'a50');

-- expected median with default collation
SELECT percentile_disc(0.5) WITHIN GROUP (ORDER BY valDefaultCollation) as expected_median
FROM textvals2;
-- actual median with default collation
SELECT median(valDefaultCollation) FROM textvals2;

-- expected median with numeric collation
SELECT percentile_disc(0.5) WITHIN GROUP (ORDER BY valNumericOrder) as expected_median
FROM textvals2;
-- actual median with numeric collation
SELECT median(valNumericOrder) FROM textvals2;

-- Test large table with text
CREATE TABLE textvalslarge (val text);

INSERT INTO textvalslarge(val)
SELECT 'TEXT-' || i
FROM generate_series(0, 1000000) as T(i);

-- expected median
SELECT percentile_disc(0.5) WITHIN GROUP (ORDER BY val) as expected_median
FROM textvalslarge;
-- actual median
SELECT median(val) FROM textvalslarge;

-- Test large table with smallint
CREATE TABLE smallintvals(val smallint);

INSERT INTO smallintvals(val)
SELECT
    CASE
        WHEN i % 2 = 0 THEN i % 32767
        ELSE -i % 32767
    END
FROM generate_series(0, 10000000) as T(i);

-- expected median
SELECT percentile_disc(0.5) WITHIN GROUP (ORDER BY val) as expected_median
FROM smallintvals;
-- actual median
SELECT median(val) FROM smallintvals;
