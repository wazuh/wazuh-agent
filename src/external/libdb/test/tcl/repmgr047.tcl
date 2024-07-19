# Copyright (c) 2016, 2020 Oracle and/or its affiliates.  All rights reserved.
#
# See the file LICENSE for license information.
#
# $Id$
#
# TEST	repmgr047
# TEST	Environment-level MVCC on a replication client.
# TEST
# TEST	Ensure that DB_MULTIVERSION creates the extra page copies
# TEST	for MVCC and that DB_TXN_SNAPSHOT operates as expected on
# TEST	a replication client.
# TEST	
# TEST	The basic test ensures the expected operation visibility
# TEST	with snapshot transactions.  There are also test cases
# TEST	for cursor position, freezing/thawing, and to make sure
# TEST	a client operates correctly when it gets the environment-
# TEST	level DB_MULTIVERSION setting from the DB_CONFIG file.
# TEST
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr047 { { niter 100 } { tnum "047" } args } {

	source ./include.tcl

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	set method "btree"
	set args [convert_args $method $args]

	puts "Repmgr$tnum ($method): Environment-level MVCC on a replication\
	    client."
	set configlist { { 1 1 1 } { 1 1 0 } { 1 0 0 } { 0 1 1 } { 0 1 0 } }
	foreach config $configlist {
		set mmvcc [lindex $config 0]
		set cmvcc [lindex $config 1]
		set c2mvcc [lindex $config 2]
		puts "Repmgr$tnum ($method): Basic test (MVCC: master $mmvcc,\
		    client $cmvcc, client2 $c2mvcc)."
		repmgr047_sub $method $niter $tnum $mmvcc $cmvcc $c2mvcc $args
	}

	puts "Repmgr$tnum ($method): Cursor adjustment test."
	repmgr047_curadj $method $niter $tnum $args

	puts "Repmgr$tnum ($method): Freeze/thaw behavior test."
	repmgr047_freezethaw $method $niter $tnum $args

	puts "Repmgr$tnum ($method): Client DB_CONFIG test."
	repmgr047_dbconfig $method $niter $tnum $args
}

#
# Test basic client MVCC functionality by checking visibility of
# master operations in client snapshot transactions.  This is an
# extension of the basic test case in test120.  This test creates
# a master and two clients and uses parameters to determine whether
# environment-level MVCC is on or off on each site.
#
proc repmgr047_sub { method niter tnum mmvcc cmvcc c2mvcc largs } {
	global testdir
	global rep_verbose
	global verbose_type
	global ipversion
	global databases_in_memory
	set nsites 3

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	set sslargs [setup_repmgr_sslargs]

	env_cleanup $testdir
	set ports [available_ports $nsites]
	set hoststr [get_hoststr $ipversion]
	set omethod [convert_method $method]

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR
	set clientdir2 $testdir/CLIENTDIR2

	file mkdir $masterdir
	file mkdir $clientdir
	file mkdir $clientdir2

	set mmvccopt ""
	set cmvccopt ""
	set c2mvccopt ""
	if { $mmvcc } {
		set mmvccopt "-multiversion"
	}
	if { $cmvcc } {
		set cmvccopt "-multiversion"
	}
	if { $c2mvcc } {
		set c2mvccopt "-multiversion"
	}

	puts "\tRepmgr$tnum.a: Start a master."
	set ma_envcmd "berkdb_env_noerr -create $verbargs $sslargs \
	    -errpfx MASTER -home $masterdir $mmvccopt \
	    -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master

	puts "\tRepmgr$tnum.b: Start two clients."
	set cl_envcmd "berkdb_env_noerr -create $verbargs $cmvccopt $sslargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -remote [list $hoststr [lindex $ports 2]] \
	    -start client
	await_startup_done $clientenv

	set cl2_envcmd "berkdb_env_noerr -create $verbargs $c2mvccopt $sslargs \
	    -errpfx CLIENT2 -home $clientdir2 -txn -rep -thread"
	set clientenv2 [eval $cl2_envcmd]
	$clientenv2 repmgr -ack all \
	    -local [list $hoststr [lindex $ports 2]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -remote [list $hoststr [lindex $ports 1]] \
	    -start client
	await_startup_done $clientenv2

	#
	# Create a new database on master and handles to the client
	# copies of that database.
	#
	if {$databases_in_memory} {
		set filename { "" "mvcctest.db" }
	} else {
		set filename  "mvcctest.db"
	}
	set mmvccdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $masterenv $largs $filename"]
	error_check_good mdb_open [is_valid_db $mmvccdb] TRUE
	set cmvccdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv $largs $filename"]
	error_check_good cdb_open [is_valid_db $cmvccdb] TRUE
	set c2mvccdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv2 $largs $filename"]
	error_check_good c2db_open [is_valid_db $c2mvccdb] TRUE

	#
	# We must manage client transactions differently depending on
	# whether the client is MVCC or non-MVCC.  MVCC reduces the need
	# for locking by using extra page copies to preserve a consistent
	# view of the data.  In the non-MVCC case, that locking is present
	# and can lock out later test steps.  Therefore, non-MVCC clients
	# need to commit and start new transactions more frequently than
	# MVCC clients to run the same sequence of test operations.
	#

	puts "\tRepmgr$tnum.c: Start first master transaction."
	set key 1
	set data DATA
	if { $mmvcc } {
		set t1m [$masterenv txn -snapshot]
	} else {
		set t1m [$masterenv txn]
	}

	puts "\tRepmgr$tnum.d: Start first client transactions."
	#
	# Start client transactions t2c(2) before t1m is replicated.
	# There is nothing for the client transactions to see yet.
	#
	if { $cmvcc } {
		set t2c [$clientenv txn -snapshot]
	} else {
		set t2c [$clientenv txn]
	}
	set ret [lindex [$cmvccdb get -txn $t2c $key] 0]
	error_check_good cmvccdb_t2get1k1 $ret ""
	if { $c2mvcc } {
		set t2c2 [$clientenv2 txn -snapshot]
	} else {
		set t2c2 [$clientenv2 txn]
	}
	set ret [lindex [$c2mvccdb get -txn $t2c2 $key] 0]
	error_check_good c2mvccdb_t2get1k1 $ret ""

	puts "\tRepmgr$tnum.e: Commit first master transaction."
	# Perform simple put and commit t1m on master.
	error_check_good t1m_put \
	    [eval $mmvccdb put -txn $t1m $key [chop_data $method $data]] 0
	error_check_good t1m_commit [$t1m commit] 0
	set ret [lindex [$mmvccdb get $key] 0]
	error_check_good mmvccdb_gett1m $ret [list $key $data]

	puts "\tRepmgr$tnum.f: First client txns shouldn't see first\
	    master txn."
	set ret [lindex [$cmvccdb get -txn $t2c $key] 0]
	error_check_good cmvccdb_t2get2k1m $ret ""
	set ret [lindex [$c2mvccdb get -txn $t2c2 $key] 0]
	error_check_good c2mvccdb_t2get2k1m $ret ""

	# For non-MVCC, must commit first client txn to avoid locking.
	if { !$cmvcc } {
		error_check_good t2c_commit [$t2c commit] 0
	}
	if { !$c2mvcc } {
		error_check_good t2c2_commit [$t2c2 commit] 0
	}

	puts "\tRepmgr$tnum.g: Start second client txns, should see first\
	    master txn."
	# Start client transactions t3c(2) after t1m is replicated.
	if { $cmvcc } {
		set t3c [$clientenv txn -snapshot]
	} else {
		set t3c [$clientenv txn]
	}
	set ret [lindex [$cmvccdb get -txn $t3c $key] 0]
	error_check_good cmvccdb_t3get1k1 $ret [list $key $data]
	if { $c2mvcc } {
		set t3c2 [$clientenv2 txn -snapshot]
	} else {
		set t3c2 [$clientenv2 txn]
	}
	set ret [lindex [$c2mvccdb get -txn $t3c2 $key] 0]
	error_check_good c2mvccdb_t3get1k1 $ret [list $key $data]

	puts "\tRepmgr$tnum.h: Start and commit second master transaction."
	set key2 2
	set data2 DATA2
	if { $mmvcc } {
		set t4m [$masterenv txn -snapshot]
	} else {
		set t4m [$masterenv txn]
	}
	error_check_good t4m_put \
	    [eval $mmvccdb put -txn $t4m $key2 [chop_data $method $data2]] 0
	error_check_good t4m_commit [$t4m commit] 0
	set ret [lindex [$mmvccdb get $key2] 0]
	error_check_good mmvccdb_gett4m $ret [list $key2 $data2]

	puts "\tRepmgr$tnum.i: First client snapshot txns shouldn't see\
	    second master txn."
	# If mvcc and we therefore still have t2c(2), can't see t4m.
	if { $cmvcc } {
		set ret [lindex [$cmvccdb get -txn $t2c $key2] 0]
		error_check_good cmvccdb_t2get1k2m $ret ""
	}
	if { $c2mvcc } {
		set ret [lindex [$c2mvccdb get -txn $t2c2 $key2] 0]
		error_check_good c2mvccdb_t2get1k2m $ret ""
	}

	puts "\tRepmgr$tnum.j: Second client txns shouldn't see second\
	    master txn."
	# t3c(2) can't see t4m.
	set ret [lindex [$cmvccdb get -txn $t3c $key2] 0]
	error_check_good cmvccdb_t3get1k2m $ret ""
	set ret [lindex [$c2mvccdb get -txn $t3c2 $key2] 0]
	error_check_good c2mvccdb_t3get1k2m $ret ""

	# For non-MVCC, must commit second client txn to avoid locking.
	if { !$cmvcc } {
		error_check_good t3c_commit [$t3c commit] 0
	}
	if { !$c2mvcc } {
		error_check_good t3c2_commit [$t3c2 commit] 0
	}

	puts "\tRepmgr$tnum.k: Start third client txns, should see second\
	    master txn."
	# Start client transactions t5c(2) after t4m is replicated.
	if { $cmvcc } {
		set t5c [$clientenv txn -snapshot]
	} else {
		set t5c [$clientenv txn]
	}
	set ret [lindex [$cmvccdb get -txn $t5c $key] 0]
	error_check_good cmvccdb_t5get1k1 $ret [list $key $data]
	set ret [lindex [$cmvccdb get -txn $t5c $key2] 0]
	error_check_good cmvccdb_t5get1k2 $ret [list $key2 $data2]
	if { $c2mvcc } {
		set t5c2 [$clientenv2 txn -snapshot]
	} else {
		set t5c2 [$clientenv2 txn]
	}
	set ret [lindex [$c2mvccdb get -txn $t5c2 $key] 0]
	error_check_good c2mvccdb_t5get1k1 $ret [list $key $data]
	set ret [lindex [$c2mvccdb get -txn $t5c2 $key2] 0]
	error_check_good c2mvccdb_t5get1k2 $ret [list $key2 $data2]

	# Clean up unresolved client transactions.
	if { $c2mvcc } {
		error_check_good t2c2_commit [$t2c2 commit] 0
		error_check_good t3c2_commit [$t3c2 commit] 0
	}
	error_check_good t5c2_commit [$t5c2 commit] 0
	if { $cmvcc } {
		error_check_good t2c_commit [$t2c commit] 0
		error_check_good t3c_commit [$t3c commit] 0
	}
	error_check_good t5c_commit [$t5c commit] 0

	puts "\tRepmgr$tnum.l: Verify client database contents."
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1 $filename
	rep_verify $masterdir $masterenv $clientdir2 $clientenv2 1 1 1 $filename

	error_check_good c2mvccdb_close [$c2mvccdb close] 0
	error_check_good cmvccdb_close [$cmvccdb close] 0
	error_check_good mmvccdb_close [$mmvccdb close] 0
	error_check_good client2_close [$clientenv2 close] 0
	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}

#
# Test that cursor adjustment works with client MVCC.  Similar test
# case to test121 except that the snapshot transaction and cursor are
# on a replication client.
#
proc repmgr047_curadj { method niter tnum largs } {
	global testdir
	global rep_verbose
	global verbose_type
	global ipversion
	global databases_in_memory
	set nsites 2

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	set sslargs [setup_repmgr_sslargs]

	env_cleanup $testdir
	set ports [available_ports $nsites]
	set hoststr [get_hoststr $ipversion]
	set omethod [convert_method $method]

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR

	file mkdir $masterdir
	file mkdir $clientdir

	#
	# It is important to make sure that master transactions are
	# fully applied on the client before checking for client even
	# and odd key values.  Therefore, use heartbeats to assure
	# rerequest processing.
	#
	set hbsend 50000
	set hbmon 110000

	puts "\tRepmgr$tnum.ca.a: Start a master."
	set ma_envcmd "berkdb_env_noerr -create $verbargs $sslargs \
	    -errpfx MASTER -home $masterdir -multiversion \
	    -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -timeout [list heartbeat_send $hbsend] \
	    -timeout [list heartbeat_monitor $hbmon] \
	    -start master

	puts "\tRepmgr$tnum.ca.b: Start a client."
	set cl_envcmd "berkdb_env_noerr -create $verbargs $sslargs -multiversion \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -timeout [list heartbeat_send $hbsend] \
	    -timeout [list heartbeat_monitor $hbmon] \
	    -start client
	await_startup_done $clientenv

	#
	# Create a new database on master and a handle to the client
	# copy of that database.
	#
	if {$databases_in_memory} {
		set filename { "" "mvcccatest.db" }
	} else {
		set filename  "mvcccatest.db"
	}
	set mmvccdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $masterenv $largs $filename"]
	error_check_good mdb_open [is_valid_db $mmvccdb] TRUE
	set cmvccdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv $largs $filename"]
	error_check_good cdb_open [is_valid_db $cmvccdb] TRUE

	puts "\tRepmgr$tnum.ca.c: Enter data for even keys on master."
	# Enter some data using txn1.  Leave holes by using keys 2, 4, 6 ...
	set t1 [$masterenv txn]
	set txn1 "-txn $t1"
	#
	# We already have the usual niter that we use elsewhere in
	# the test.  Here we need a different, much larger iterator
	# to create a large enough test case for cursor adjustment.
	#
	set bigiter 10000
	set data DATA
	for { set i 1 } { $i <= $bigiter } { incr i } {
		set key [expr $i * 2]
		error_check_good t1_put [eval {$mmvccdb put} \
		    $txn1 $key $data.$key] 0
	}
	error_check_good t1_commit [$t1 commit] 0

	#
	# We need to assure that the entire even key master transaction
	# is applied before creating the client snapshot transaction.
	# Slower or more fully loaded machines are more prone to gaps
	# or just taking a longer time.  Therefore, loop to insert
	# unrelated records and check if client apply has caught up to
	# the even key master transaction.
	#
	set mas_mpl [stat_field $masterenv rep_stat "Maximum permanent LSN"]
	set mas_mplf [lindex $mas_mpl 0]
	set mas_mplo [lindex $mas_mpl 1]
	set even_applied 0
	set start 0
	while { !$even_applied } {
		eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
		incr start $niter
		set cli_mpl \
		    [stat_field $clientenv rep_stat "Maximum permanent LSN"]
		set cli_mplf [lindex $cli_mpl 0]
		set cli_mplo [lindex $cli_mpl 1]
		if { $cli_mplf > $mas_mplf || \
		    ($cli_mplf == $mas_mplf && $cli_mplo >= $mas_mplo)} {
			set even_applied 1
		}
	}

	puts "\tRepmgr$tnum.ca.d: Start client snapshot transaction and cursor."
	set t2 [$clientenv txn -snapshot]
	set txn2 "-txn $t2"
	set cursor [eval {$cmvccdb cursor} $txn2]
	error_check_good cdb_cursor [is_valid_cursor $cursor $cmvccdb] TRUE

	puts "\tRepmgr$tnum.ca.e: Position client cursor halfway through data."
	# Walk the cursor halfway through the database.
	set i 1
	set halfway [expr $bigiter / 2]
	for { set ret [$cursor get -first] } \
	    { $i <= $halfway } \
	    { set ret [$cursor get -next] } {
		incr i
	}
	set currentkey [lindex [lindex $ret 0] 0]
	set currentdata [lindex [lindex $ret 0] 1]

	# Start a new master transaction and use it to enter more data.
	# Verify that the client cursor is not changed.
	puts "\tRepmgr$tnum.ca.f: Enter data for odd keys on master."
	set t3 [$masterenv txn]
	set txn3 "-txn $t3"

	# Enter more data, filling in the holes from the first time around by
	# using keys 1, 3, 5 ...  Client cursor should stay on the same item.
	for { set i 1 } { $i <= $bigiter } { incr i } {
		set key [expr [expr $i * 2] - 1]
		error_check_good t3_put [eval {$mmvccdb put} \
		    $txn3 $key $data.$key] 0
	}
	error_check_good t3_commit [$t3 commit] 0

	# Wait for odd key master transaction to be applied.
	set mas_mpl [stat_field $masterenv rep_stat "Maximum permanent LSN"]
	set mas_mplf [lindex $mas_mpl 0]
	set mas_mplo [lindex $mas_mpl 1]
	set odd_applied 0
	while { !$odd_applied } {
		eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
		incr start $niter
		set cli_mpl \
		    [stat_field $clientenv rep_stat "Maximum permanent LSN"]
		set cli_mplf [lindex $cli_mpl 0]
		set cli_mplo [lindex $cli_mpl 1]
		if { $cli_mplf > $mas_mplf || \
		    ($cli_mplf == $mas_mplf && $cli_mplo >= $mas_mplo)} {
			set odd_applied 1
		}
	}

	puts "\tRepmgr$tnum.ca.g: Make sure odd keys are replicated to client."
	set ret [lindex [$cmvccdb get $key] 0]
	error_check_good cmvccdb_gett3 $ret [list $key $data.$key]

	puts "\tRepmgr$tnum.ca.h: Verify client cursor position is unchanged."
	set ret [$cursor get -current]
	set k [lindex [lindex $ret 0] 0]
	set d [lindex [lindex $ret 0] 1]
	error_check_good current_key $k $currentkey
	error_check_good current_data $d $currentdata

	error_check_good cursor_close [$cursor close] 0
	error_check_good t2_commit [$t2 commit] 0
	error_check_good cmvccdb_close [$cmvccdb close] 0
	error_check_good mmvccdb_close [$mmvccdb close] 0
	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}

#
# Test freeze/thaw behavior with client MVCC.  Similar test case
# to test154 except that the snapshot transaction and cursor are
# on a replication client.
#
proc repmgr047_freezethaw { method niter tnum largs } {
	source ./include.tcl
	global testdir
	global rep_verbose
	global verbose_type
	global ipversion
	global databases_in_memory
	set nsites 2

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	set sslargs [setup_repmgr_sslargs]

	env_cleanup $testdir
	set ports [available_ports $nsites]
	set hoststr [get_hoststr $ipversion]
	set omethod [convert_method $method]

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR

	file mkdir $masterdir
	file mkdir $clientdir

	set args [convert_args $method $largs]
	set omethod [convert_method $method]
	set encargs ""
	set args [split_encargs $args encargs]
	set pageargs ""
	split_pageargs $args pageargs
	set filename "test$tnum.db"

	#
	# When native pagesize is small, we need to adjust the 
	# default numbers of locks and mutexes.
	#
	set mutexargs ""
	set max_locks 2000
	set max_objects 2000
	set native_pagesize [get_native_pagesize]
	if { $native_pagesize < 2048 } { 
		set mutexargs "-mutex_set_max 40000"
		set max_locks 5000
		set max_objects 5000
	}

	#
	# We need a tiny cachesize in order to generate freezer files,
	# but it needs some adjustment with various pagesizes in order to
	# avoid running out of memory.
	#
	set cachesize [expr 65536]
	set pagesize 0
	set pgindex [lsearch -exact $args "-pagesize"]
	if { $pgindex != -1 } {
		incr pgindex
		set pagesize [lindex $args $pgindex]
	}
	if {$pagesize == 512} {
		puts "\tRepmgr$tnum.ft skipping freeze/thaw for pagesize 512."
		return
	}
	if {$pagesize > 8192 || $native_pagesize > 8192} {
		set cachesize [expr 262144]
	}
	set cacheargs "-cachesize { 0 $cachesize 1 }"
	
	puts "\tRepmgr$tnum.ft.a: Start a master."
	set ma_envcmd "berkdb_env_noerr -create $verbargs $sslargs \
	    $cacheargs $pageargs $encargs \
	    -lock_max_locks $max_locks -lock_max_objects $max_objects \
	    -errpfx MASTER -home $masterdir -multiversion \
	    -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master

	puts "\tRepmgr$tnum.ft.b: Start a client."
	set cl_envcmd "berkdb_env_noerr -create $verbargs $sslargs -multiversion \
	    $cacheargs $pageargs $encargs \
	    -lock_max_locks $max_locks -lock_max_objects $max_objects \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	await_startup_done $clientenv

	#
	# Create a new database on master and a handle to the client
	# copy of that database.
	#
	if {$databases_in_memory} {
		set filename { "" "mvccfttest.db" }
	} else {
		set filename  "mvccfttest.db"
	}
	set mmvccdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $masterenv $largs $filename"]
	error_check_good mdb_open [is_valid_db $mmvccdb] TRUE
	set cmvccdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv $largs $filename"]
	error_check_good cdb_open [is_valid_db $cmvccdb] TRUE

	#
	# Go through a loop of starting a client snapshot txn, making many
	# entries in a master writer txn, and verifying that the client 
	# snapshot does not see the writer's changes after they are
	# replicated.
	#
	set iter 5
	set nentries 1000
	set did [open $dict]
	for { set i 0 } { $i < $iter } { incr i } {
		puts "\tRepmgr$tnum.ft.c1: Iteration $i:\
		    start snapshot transaction."
		set snap_count 0
		set t0 [$clientenv txn -snapshot]
		set snapshot_txn "-txn $t0"	
		set snapshot_cursor [eval {$cmvccdb cursor} $snapshot_txn]
	
		#
		# Read through the db with a snapshot cursor to 
		# find out how many entries it sees.
		#
		for { set dbt [$snapshot_cursor get -first] }\
		    { [llength $dbt] != 0 } \
		    { set dbt [$snapshot_cursor get -next] } { 
			incr snap_count
		}
		set before_writes $snap_count

		puts "\tRepmgr$tnum.ft.c2: Iteration $i:\
		    start writer transaction."
		set t1 [$masterenv txn]
		set writer_txn "-txn $t1"
	
		# Enter some data using writer transaction. 
		set write_count 0
		while { [gets $did str] != -1 && $write_count < $nentries } {
                        if { [is_record_based $method] == 1 } {
                                set key [expr $i * $nentries + $write_count + 1]
                        } else {
                                set key $str
                        }

                        set ret [eval \
                            {$mmvccdb put} $writer_txn \
                            {$key [chop_data $method $str]}]
                        error_check_good put $ret 0
                        incr write_count
		}
		# Commit master txn here so that it is replicated to client.
		error_check_good writer_txn_commit [$t1 commit] 0

		# Snapshot txn counts its view again.
		set snap_count 0
		for { set dbt [$snapshot_cursor get -first] }\
		    { [llength $dbt] != 0 } \
		    { set dbt [$snapshot_cursor get -next] } { 
			incr snap_count
		}
		set after_writes $snap_count

		puts "\tRepmgr$tnum.ft.c3: Iteration $i: check data."
		#
		# On each iteration, the snapshot will find more entries, but
		# not within an iteration.
		#
		error_check_good total_count $snap_count \
		    [expr $i * $write_count]
		error_check_good snap_count_unchanged \
		    $before_writes $after_writes

		error_check_good snapshot_txn_commit [$t0 commit] 0	

		tclsleep 15	
	}

	puts "\tRepmgr$tnum.ft.d: Check mpool freeze/thaw stats."
	error_check_good client_frozen [expr \
	    [stat_field $clientenv mpool_stat "Buffers frozen"] > 0] 1
	error_check_good client_thawed [expr \
	    [stat_field $clientenv mpool_stat "Buffers thawed"] > 0] 1

	error_check_good cmvccdb_close [$cmvccdb close] 0
	error_check_good mmvccdb_close [$mmvccdb close] 0
	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}

#
# Make sure that client MVCC behavior can be obtained by setting
# DB_MULTIVERSION in the DB_CONFIG file.  This is the only way
# available to BDB SQL and BDB XML users to use client MVCC.
#
proc repmgr047_dbconfig { method niter tnum largs } {
	global testdir
	global rep_verbose
	global verbose_type
	global ipversion
	global databases_in_memory
	set nsites 2

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	set sslargs [setup_repmgr_sslargs]

	env_cleanup $testdir
	set ports [available_ports $nsites]
	set hoststr [get_hoststr $ipversion]
	set omethod [convert_method $method]

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR

	file mkdir $masterdir
	file mkdir $clientdir

	puts "\tRepmgr$tnum.dbc.a: Create client DB_CONFIG specifying\
	    DB_MULTIVERSION."
	set cid [open $clientdir/DB_CONFIG w]
	puts $cid "set_flags DB_MULTIVERSION"
	close $cid

	puts "\tRepmgr$tnum.dbc.b: Start a master."
	set ma_envcmd "berkdb_env_noerr -create $verbargs $sslargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master

	puts "\tRepmgr$tnum.dbc.c: Start a client without DB_MULTIVERSION."
	set cl_envcmd "berkdb_env_noerr -create $verbargs $sslargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	await_startup_done $clientenv

	#
	# Create a new database on master and handles to the client
	# copies of that database.
	#
	if {$databases_in_memory} {
		set filename { "" "mvccdbconfig.db" }
	} else {
		set filename  "mvccdbconfig.db"
	}
	set mmvccdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $masterenv $largs $filename"]
	error_check_good mdb_open [is_valid_db $mmvccdb] TRUE
	set cmvccdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv $largs $filename"]
	error_check_good cdb_open [is_valid_db $cmvccdb] TRUE

	puts "\tRepmgr$tnum.dbc.d: Start master write transaction."
	set key 1
	set data DATA
	set t1 [$masterenv txn]
	set txn1 "-txn $t1"
	puts "\tRepmgr$tnum.dbc.e: Start first client snapshot transaction."
	set t2 [$clientenv txn -snapshot]
	set txn2 "-txn $t2"
	set ret [eval {$cmvccdb get} $txn2 $key]
	error_check_good txn2_get [llength $ret] 0

	puts "\tRepmgr$tnum.dbc.f: Put data and commit master transaction."
	error_check_good \
	    t1_put [eval {$mmvccdb put} $txn1 $key [chop_data $method $data]] 0
	error_check_good t1_commit [$t1 commit] 0

	puts "\tRepmgr$tnum.dbc.g: First client snapshot can't see master put."
	set ret [eval {$cmvccdb get} $txn2 $key]
	error_check_good txn2_get [llength $ret] 0

	puts "\tRepmgr$tnum.dbc.h: A new client snapshot can see master put."
	set t3 [$clientenv txn -snapshot]
	set txn3 "-txn $t3"
	set ret [eval {$cmvccdb get} $txn3 $key]
	error_check_good \
	    t3_get $ret [list [list $key [pad_data $method $data]]]

	error_check_good t2_commit [$t2 commit] 0
	error_check_good t3_commit [$t3 commit] 0

	puts "\tRepmgr$tnum.dbc.i: Verify client database contents."
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1 $filename

	error_check_good cmvccdb_close [$cmvccdb close] 0
	error_check_good mmvccdb_close [$mmvccdb close] 0
	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}
