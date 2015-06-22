.bail on


/****** Table Statuses ******
 *
 * Test statuses & descriptions
 */
DROP TABLE IF EXISTS Statuses;
CREATE TABLE Statuses (
	id			integer			primary key,
	desc		varchar(16)
);

begin transaction;
INSERT INTO Statuses VALUES (1, 'not started');
INSERT INTO Statuses VALUES (2, 'running');
INSERT INTO Statuses VALUES (3, 'stopped');
INSERT INTO Statuses VALUES (4, 'failed');
INSERT INTO Statuses VALUES (5, 'completed');
commit;



/****** Table Types ******
 *
 * Types of tests
 */
DROP TABLE IF EXISTS Types;
CREATE TABLE Types (
	id			integer			primary key,
	name		varchar(32)		not null,
	desc		varchar(255)
);

begin transaction;
INSERT INTO Types VALUES (
	1,
	'qsort_validate',
	'validate correctness of behavior'
);
INSERT INTO Types VALUES (
	2,
	'qsort_benchmark',
	'test performance'
);
INSERT INTO Types VALUES (
	3,
	'qsort_size',
	'instantiate template and output function size'
);
commit;


/****** Table TestSets ******
 *
 * Each row is a single set of tests to be run
 */
DROP INDEX IF EXISTS idxTestSetsType;
DROP INDEX IF EXISTS idxTestSetsStatus;

DROP TABLE IF EXISTS TestSets;
CREATE TABLE TestSets (
	id			integer			primary key AUTOINCREMENT,
	type		integer			not null,
	name        varchar(64) 	not null,
	desc        varchar(255),
	params		text,
	status		integer			not null,


	FOREIGN KEY(type)		REFERENCES Types(id),
	FOREIGN KEY(status)		REFERENCES Statuses(id),
	CONSTRAINT uniqueTestSetName UNIQUE (name) ON CONFLICT FAIL
);
CREATE INDEX idxTestSetsType on TestSets (type);
CREATE INDEX idxTestSetsStatus on TestSets (status);



DROP INDEX IF EXISTS idxCompilersSet;
DROP INDEX IF EXISTS idxCompilersTestId;

/****** Table Compilers ******
 *
 * Each row represents a unique combination of compiler, version target, host
 * and relevant environment variables (CFLAGS, CPPFLAGS, PATH,etc.)
 * combination.
 */
DROP TABLE IF EXISTS Compilers;
CREATE TABLE Compilers (
	id			integer			primary key AUTOINCREMENT,
	testSetId	integer			not null,
/*	vendor		varchar(64)		not null,*/
	name		varchar(64)		not null,
	version		varchar(64)		not null,
	host		varchar(128),
	target		varchar(128),
	cc			varchar(255),
	env			text,

	FOREIGN KEY(testSetId) REFERENCES TestSets(testSetId)
);

CREATE INDEX idxCompilersSet on Compilers (
	name, version
);
CREATE INDEX idxCompilersTestId on Compilers (testSetId);



/****** Table QsortVariants ******
 *
 *
 */
DROP INDEX IF EXISTS idxQsortVariantsTestId;

DROP TABLE IF EXISTS QsortVariants;
CREATE TABLE QsortVariants (
	id				integer			primary key AUTOINCREMENT,
	testSetId		integer			not null,
	dataSize		integer			not null,
	signedKey		bool			not null,
	n				integer			not null,
	elemSize		integer			not null,
	align			integer			not null,
	less_fn			varchar(64)		not null,
	outlineCopy		bool			not null,
	outlineSwap		bool			not null,
	supplyBuffer	bool			not null,
	maxSizeBits		bool			not null,
	maxThresh		integer			not null,

	FOREIGN KEY(testSetId) REFERENCES TestSets(testSetId),
	CONSTRAINT uniqueVariants UNIQUE (
		testSetId, dataSize, signedKey, n, elemSize, align, less_fn,
		outlineCopy, outlineSwap, supplyBuffer, maxSizeBits,
		maxThresh
	) ON CONFLICT FAIL
);
CREATE INDEX idxQsortVariantsTestId on QsortVariants (testSetId);


/****** Table QsortResults ******
 *
 *
 */
DROP TABLE IF EXISTS QsortResults;
CREATE TABLE QsortResults (
	id					integer			primary key AUTOINCREMENT,
	testSetId			integer			not null,
	compilerId			integer			not null,
	variantId			integer			not null,
	status				integer			not null,
	validated			integer,
	startTime			integer,
	endTime				integer,
	fnSize				integer,
	count0				integer,
	count1				integer,
	count2				integer,
	time0				real,
	time1				real,
	time2				real,
	ips0				real,
	ips1				real,
	ips2				real,

	FOREIGN KEY(testSetId)	REFERENCES TestSets(id),
	FOREIGN KEY(compilerId)	REFERENCES Compilers(id),
	FOREIGN KEY(variantId)	REFERENCES QsortVariants(id)
);

DROP VIEW IF EXISTS Results;
CREATE VIEW Results AS
SELECT
	r.id,
	r.testSetId,
	r.compilerId,
	r.variantId,
	v.elemSize,
	v.align,
	v.signedKey,
	v.less_fn,
	v.outlineCopy,
	v.outlineSwap,
	v.supplyBuffer,
	v.maxSizeBits,
	v.maxThresh,
	v.dataSize,
	c.version,
	r.status,
	r.validated,
	r.fnSize,
	r.ips2 / r.ips0 as overQsort,
	r.ips2 / r.ips1 as overMsort
FROM
	(QsortResults as r inner join QsortVariants as v
		on r.variantId = v.id)
	inner join Compilers as c on r.compilerId = c.id
WHERE
	status > 1;

/****** Table xxxxxxxxxxx ******
 *
 *
 * /
DROP INDEX IF EXISTS iiiiiiiiiii;
DROP TABLE IF EXISTS xxxxxxxxxxx;
CREATE TABLE xxxxxxxxxxx (
	id			integer			primary key,
);

CREATE INDEX iiiiiiiiiii on xxxxxxxxxxx (

);
*/

