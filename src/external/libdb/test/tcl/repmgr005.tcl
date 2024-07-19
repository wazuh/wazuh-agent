# Copyright (c) 2016, 2020 Oracle and/or its affiliates.  All rights reserved.
#
# See the file LICENSE for license information.
#
# $Id$
#
# TEST	repmgr005
# TEST	Test use of set_memory_init() with replication.
# TEST
# TEST	Tests the set_memory_init() object types that are used to
# TEST	calculate the additional environment region memory needed
# TEST	for replication:  DB_MEM_DATABASE, DB_MEM_DATABASE_LENGTH,
# TEST	DB_MEM_EXTFILE_DATABASE, DB_MEM_REP_SITE.
# TEST
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr005 { { niter 100 } { tnum "005" } args } {

	source ./include.tcl

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	set method "btree"
	set args [convert_args $method $args]

	# Run test using ondisk and private environment cases.
	set envopts { ondisk private }
	foreach e $envopts {
		set primsg ""
		if { $e == "private" } {
			set primsg " in private env"
		}
		puts "Repmgr$tnum ($method): set_memory_init()\
		    with replication$primsg."
		repmgr005_sub $method $niter $tnum $args $e
	}
}

proc repmgr005_sub { method niter tnum largs envopt } {
	global testdir

	env_cleanup $testdir

	set defaultdir $testdir/DEFAULTDIR
	set nonrepdir $testdir/NONREPDIR

	file mkdir $defaultdir
	file mkdir $nonrepdir

	set private ""
	if { $envopt == "private" } {
		set private "-private"
	}

	puts "\tRepmgr$tnum.a: Create a default environment using replication."
	# Use defaults for set_memory_init() values being tested.
	set df_envcmd "berkdb_env_noerr -create \
	    -errpfx DEFAULT -home $defaultdir -txn -rep -thread $private"
	set defaultenv [eval $df_envcmd]
	puts "\tRepmgr$tnum.a.1: Check for expected set_memory_init() values."
	# Expected values are all 0 because nothing was explicitly configured.
	error_check_good ddbchk [$defaultenv get_database] 0
	error_check_good ddblchk [$defaultenv get_database_len] 0
	error_check_good defdchk [$defaultenv get_extfile_db] 0
	error_check_good drschk [$defaultenv get_rep_site] 0
	# Preserve default replication env region size for later comparisons.
	set defaultregsize [get_env_region_size $defaultenv $defaultdir]
	error_check_good default_close [$defaultenv close] 0

	puts "\tRepmgr$tnum.b: Create non-replication environment."
	# Set non-replication initial values.
	set nr_database 200
	set nr_database_len 300
	set nr_extfile_db 1000
	set nr_rep_site 16
	#
	# We can't stop someone from defining -rep_site here because we
	# don't yet know that the environment won't use -rep.  Use this
	# test case to prove it doesn't blow anything up.
	#
	set nr_envcmd "berkdb_env_noerr -create \
	    -errpfx NONREP -errfile $testdir/rm5nonrep.err -home $nonrepdir \
	    -txn -thread -database $nr_database -database_len $nr_database_len \
	    -extfile_db $nr_extfile_db -rep_site $nr_rep_site $private"
	set nonrepenv [eval $nr_envcmd]
	puts "\tRepmgr$tnum.b.1: Check for expected set_memory_init() values."
	error_check_good nrdb [$nonrepenv get_database] $nr_database
	error_check_good nrdbl [$nonrepenv get_database_len] $nr_database_len
	error_check_good nrefd [$nonrepenv get_extfile_db] $nr_extfile_db
	puts "\tRepmgr$tnum.b.2: Test expected error getting -rep_site value."
	catch {[$nonrepenv get_rep_site]} res
	error_check_good nrerrchk [is_substr $res \
	    "get_rep_site:invalid argument"] 1
	error_check_good nonrep_close [$nonrepenv close] 0
	#
	# We check the errfile after closing the env because the close
	# guarantees all messages are flushed to disk.
	#	
	set nrerrfile [open $testdir/rm5nonrep.err r]
	set nrerr [read $nrerrfile]
	close $nrerrfile
	error_check_good nrmsgchk [is_substr $nrerr \
	    "configured for the replication subsystem"] 1

	puts "\tRepmgr$tnum.c: Small replication test case."
	repmgr005_initmanydbs $method $niter $tnum $largs \
	    30 75 38 7 1 20 $defaultregsize $envopt "c"

	puts "\tRepmgr$tnum.d: Medium customer-inspired replication test case."
	repmgr005_initmanydbs $method $niter $tnum $largs \
	    200 100 10 2 1 20 $defaultregsize $envopt "d"

	# Windows has a maximum path length of 260 in most cases
	# so we set our longest database name to 170, allowing 90 
	# characters for the rest of the path in our test environment.
	puts "\tRepmgr$tnum.e: Large replication test case."
	repmgr005_initmanydbs $method $niter $tnum $largs \
	    400 170 200 5 2 30 $defaultregsize $envopt "e"
}

#
# Create a replication group containing a master and a single client
# with configurable set_memory_init() objects:
#     db - number of databases
#     dblen - length of database directory/name string
#     extdb - number of additional blob metadata databases
#     site - number of sites
# and other configurable items:
#     cachemeg - size of cache in megabytes
#     clistart - seconds to wait for client startup
#     minregsize - minimum region size for comparison tests
#     envopt - ondisk or private
#     tl - test letter for output
#
# This procedure creates db databases with generated names of dblen length
# on the master and then starts the client to cause an internal init using
# many databases.  For more than a few databases, we need a larger cache
# and we need to allow more time for the client to start up.
#
# The caller should pass in a minregsize value that is the "Current region
# size" of an environment that uses replication but does not define any
# set_memory_init() objects.  This procedure does a very simple comparison
# to make sure its environments are larger than this minregsize.
#
proc repmgr005_initmanydbs { method niter tnum largs \
    db dblen extdb site cachemeg clistart minregsize envopt tl} {
	global testdir
	global rep_verbose
	global verbose_type
	global ipversion
	set nsites 2
	set omethod [convert_method $method]

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	env_cleanup $testdir
	set ports [available_ports $nsites]
	set hoststr [get_hoststr $ipversion]

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR

	file mkdir $masterdir
	file mkdir $clientdir

	setup_repmgr_ssl $masterdir
	setup_repmgr_ssl $clientdir

	set private ""
	if { $envopt == "private" } {
		set private "-private"
	}
	# Need a larger cache size for more than a few databases.
	set cachesize [expr $cachemeg * (1024 * 1024)]
	set cacheargs "-cachesize { 0 $cachesize 1 }"

	puts "\tRepmgr$tnum.$tl.1: Start a master."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread \
	    -database $db -database_len $dblen \
	    -extfile_db $extdb -rep_site $site $cacheargs $private"
	set masterenv [eval $ma_envcmd]
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master
	# Check for expected set_memory_init() values.
	error_check_good masdb [$masterenv get_database] $db
	error_check_good masdbl [$masterenv get_database_len] $dblen
	error_check_good masefd [$masterenv get_extfile_db] $extdb
	error_check_good masrs [$masterenv get_rep_site] $site
	puts "\tRepmgr$tnum.$tl.2: Check master region is larger than default."
	set masregsize [get_env_region_size $masterenv $masterdir]
	error_check_good masregsize [expr $masregsize > $minregsize] 1

	puts "\tRepmgr$tnum.$tl.3: Create $db databases with name\
	    length $dblen on master."
	set dbprefix "rm005"
	set dbsuffix ".db"
	# Inserting 6-digit numbers into the generated database names.
	set dbdigits 6
	set filllength [expr $dblen - $dbdigits - \
	    [string length $dbprefix] - [string length $dbsuffix]]
	set fillstring [string repeat "x" $filllength]
	for { set i 1 } { $i <= $db } { incr i } {
		set dbname [format "%s%s%06u%s" $dbprefix \
		    $fillstring $i $dbsuffix]
		set mdb [eval "berkdb_open_noerr -create $omethod \
		    -auto_commit -env $masterenv $largs $dbname"]
		error_check_good mdb_close [$mdb close] 0
	}

	puts "\tRepmgr$tnum.$tl.4: Start a client."
	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread \
	    -database $db -database_len $dblen \
	    -extfile_db $extdb -rep_site $site $cacheargs $private"
	set clientenv [eval $cl_envcmd]
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	await_startup_done $clientenv $clistart
	# Check for expected set_memory_init() values.
	error_check_good clidb [$clientenv get_database] $db
	error_check_good clidbl [$clientenv get_database_len] $dblen
	error_check_good cliefd [$clientenv get_extfile_db] $extdb
	error_check_good clirs [$clientenv get_rep_site] $site
	puts "\tRepmgr$tnum.$tl.5: Check client region is larger than default."
	set cliregsize [get_env_region_size $clientenv $clientdir]
	error_check_good cliregsize [expr $cliregsize > $minregsize] 1

	#
	# Use of -ack all guarantees replication complete before repmgr send
	# function returns and rep_test finishes.
	#
	puts "\tRepmgr$tnum.$tl.6: Run a set of transactions at master."
	set start 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.$tl.7: Verify client database contents."
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}

#
# Use the dbenv->stat_print() command to dump the environment statistics
# to a file and scan the file for the current region size, represented
# as megabyte and kilobyte values.  This approach works for both ondisk
# and private environments.
#
# Returns a single integer value suitable for comparison.
#
proc get_env_region_size { env dir } {

	set regmb 0
	set regkb 0

	#
	# Dump the environment stats to a file.  Note that this is a large
	# chunk of text output rather than the individual stat values we
	# expect from other subsystems.
	#
	file delete $dir/envstatprint.out
	error_check_good "$env set msgfile" \
	    [$env msgfile $dir/envstatprint.out] 0
	set ret [catch {eval $env stat_print} res]
	set statfile [open $dir/envstatprint.out r]
	set statoutput [read $statfile]
	close $statfile

	#
	# Scan env stat output for the MB and KB integer values
	# preceding "Current region size".
	#
	regexp {([0-9]*)MB *([0-9]*)KB\tCurrent region size} $statoutput \
	    match regmb regkb
	return [expr ($regmb * 1000000) + ($regkb * 1000)]
}
