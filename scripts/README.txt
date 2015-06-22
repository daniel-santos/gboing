These scripts are for running automated performance and correctness tests on
gboing libraries. Currently, the qsort template is the only supported test.
While complex, it allows larget test sets to be configured and then run at
varying times. Since very large test sets can take days, this system allows
you to stop and then restart where you left off later.

See the following for examples on how to build a test configuration:
compilers-example.conf
qsort-example.conf

Basic usage:

scripts/add_test_set.sh -d ~/my_tests.db -c scripts/compilers.conf.example \
	-t scripts/qsort.conf.example testSet1 qsort

scripts/run_test_set.sh -d ~/my_tests.db testSet1


sent SIGINT (CTRL-C) any time you want to interrupt tests and re-run
run_test_set.sh at any time to resume.


