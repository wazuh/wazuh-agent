# Copyright (c) 2007, 2020 Oracle and/or its affiliates.  All rights reserved.
#
# See the file LICENSE for license information.
#
# $Id$
#
# TEST	repmgr009
# TEST	repmgr API error test.
# TEST
# TEST	Try a variety of repmgr calls that result in errors. Also
# TEST	try combinations of repmgr and base replication API calls
# TEST	that result in errors.
# TEST
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr009 { { niter 10 } { tnum "009" } args } {

	source ./include.tcl

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	set method "btree"
	set args [convert_args $method $args]

	puts "Repmgr$tnum ($method): repmgr API error test."
	repmgr009_sub $method $niter $tnum $args

	puts "Repmgr$tnum ($method): repmgr INELECT error test."
	repmgr009_inelect $method $niter $tnum $args
}

proc repmgr009_sub { method niter tnum largs } {
	global testdir
	global rep_verbose
	global verbose_type
	global ipversion
	set nsites 2

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	env_cleanup $testdir
	set ports [available_ports [expr $nsites * 5]]
	set hoststr [get_hoststr $ipversion]

	set sslargs [setup_repmgr_sslargs]

	replsetup $testdir/MSGQUEUEDIR

	set masterdir $testdir/MASTERDIR
	set masterdir2 $testdir/MASTERDIR2
	set clientdir $testdir/CLIENTDIR
	set norepdir $testdir/NOREPDIR

	file mkdir $masterdir
	file mkdir $masterdir2
	file mkdir $clientdir
	file mkdir $norepdir

	puts "\tRepmgr$tnum.a: Set up environment without repmgr."
	set ma_envcmd "berkdb_env_noerr -create $verbargs $sslargs \
	    -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	error_check_good masterenv_close [$masterenv close] 0

	puts "\tRepmgr$tnum.b: Call repmgr without open master (error)."
	catch {$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master} res
	error_check_good errchk [is_substr $res "invalid command"] 1

	puts "\tRepmgr$tnum.c: Call repmgr_stat without open master (error)."
	catch {[stat_field $masterenv repmgr_stat "Connections dropped"]} res
	error_check_good errchk [is_substr $res "invalid command"] 1

	puts "\tRepmgr$tnum.d1: Call repmgr with 0 msgth (error)."
	set masterenv [eval $ma_envcmd]
	catch {$masterenv repmgr -start master -msgth 0 \
	    -local [list $hoststr [lindex $ports 0]]} res
	error_check_good no_threads [is_substr $res "nthreads parameter"] 1
	error_check_good allow_msgth_nonzero [$masterenv repmgr \
	    -start master -local [list $hoststr [lindex $ports 0]]] 0
	error_check_good masterenv_close [$masterenv close] 0

	puts "\tRepmgr$tnum.d2: Call repmgr with no startup flags (error)."
	set masterenv [eval $ma_envcmd]
	catch {$masterenv repmgr -start none \
	    -local [list $hoststr [lindex $ports 0]]} res
	error_check_good no_flags [is_substr $res "non-zero flags value"] 1
	error_check_good masterenv_close [$masterenv close] 0

	puts "\tRepmgr$tnum.e: Start a master with repmgr."
	repladd 1
	set masterenv [eval $ma_envcmd]
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master

	puts "\tRepmgr$tnum.f: Start repmgr with no local sites (error)."
	set cl_envcmd "berkdb_env_noerr -create $verbargs $sslargs \
	    -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	catch {$clientenv repmgr -ack all \
	    -remote [list $hoststr [lindex $ports 7]] \
	    -start client} res
	error_check_good errchk [is_substr $res \
	    "local site must be named before calling repmgr_start"] 1
	error_check_good client_close [$clientenv close] 0

	puts "\tRepmgr$tnum.g: Start repmgr with two local sites (error)."
	set clientenv [eval $cl_envcmd]
	catch {$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 8]] \
	    -local [list $hoststr [lindex $ports 9]] \
	    -start client} res
	error_check_good errchk [string match "*already*set*" $res] 1
	error_check_good client_close [$clientenv close] 0

	puts "\tRepmgr$tnum.h: Start a client."
	repladd 2
	set clientenv [eval $cl_envcmd -recover]
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	await_startup_done $clientenv

	puts "\tRepmgr$tnum.i: Start repmgr a second time (error)."
	catch {$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client} res
	error_check_good errchk [is_substr $res "repmgr is already started"] 1

	puts "\tRepmgr$tnum.j: Call rep_start after starting repmgr (error)."
	catch {$clientenv rep_start -client} res
	error_check_good errchk [is_substr $res \
	    "cannot call from Replication Manager application"] 1

	puts "\tRepmgr$tnum.k: Call rep_process_message (error)."
	set envlist "{$masterenv 1} {$clientenv 2}"
	catch {$clientenv rep_process_message 0 0 0} res
	error_check_good errchk [is_substr $res \
	    "cannot call from Replication Manager application"] 1

	#
	# Use of -ack all guarantees replication complete before repmgr send
	# function returns and rep_test finishes.
	#
	puts "\tRepmgr$tnum.l: Run some transactions at master."
	eval rep_test $method $masterenv NULL $niter $niter 0 0 $largs

	puts "\tRepmgr$tnum.m: Call rep_elect (error)."
	catch {$clientenv rep_elect 2 2 2 5000000} res
	error_check_good errchk [is_substr $res \
	    "cannot call from Replication Manager application"] 1

	puts "\tRepmgr$tnum.n: Change elect_loglength after rep_start (error)."
	catch {$clientenv rep_config {electloglength on}} res
	error_check_good elerrchk [is_substr $res \
	    "ELECT_LOGLENGTH must be configured"] 1

	puts "\tRepmgr$tnum.o: Verifying client database contents."
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0

	# We can't use sslargs with base API. We get an error.
	puts "\tRepmgr$tnum.p: Start a master with base API rep_start."
	set ma_envcmd2 "berkdb_env_noerr -create \
	    -home $masterdir2 $verbargs -errpfx MASTER -txn -thread -rep_master \
	    -rep_transport \[list 1 replsend\]"
	set masterenv2 [eval $ma_envcmd2]

	puts "\tRepmgr$tnum.q: Call repmgr after rep_start (error)."
	catch {$masterenv2 repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master} res
	# Internal repmgr calls return EINVAL after hitting
	# base API application test.
	error_check_good errchk [is_substr $res "invalid argument"] 1

	error_check_good masterenv_close [$masterenv2 close] 0

	puts "\tRepmgr$tnum.r: Start an env without starting rep or repmgr."
	set norep_envcmd "berkdb_env_noerr -create $verbargs \
	    -home $norepdir -errpfx NOREP -txn -thread \
	    -rep_transport \[list 1 replsend\]"
	set norepenv [eval $norep_envcmd]

	puts "\tRepmgr$tnum.s: Call rep_elect before rep_start (error)."
	catch {$norepenv rep_elect 2 2 2 5000000} res
	# Internal rep_elect call returns EINVAL if rep_start has not 
	# been called first.
	error_check_good errchk [is_substr $res "invalid argument"] 1

	error_check_good norepenv_close [$norepenv close] 0
	replclose $testdir/MSGQUEUEDIR
}

#
# Test the DB_REP_INELECT message by attempting to change a site's priority
# during an election.
#
proc repmgr009_inelect { method niter tnum largs } {
	global testdir
	global rep_verbose
	global verbose_type
	global ipversion
	set nsites 4
	set omethod [convert_method $method]

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	env_cleanup $testdir
	set hoststr [get_hoststr $ipversion]
	set ports [available_ports $nsites]

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR
	set clientdir2 $testdir/CLIENTDIR2
	set clientdir3 $testdir/CLIENTDIR3

	file mkdir $masterdir
	file mkdir $clientdir
	file mkdir $clientdir2
	file mkdir $clientdir3

	setup_repmgr_ssl $masterdir
	setup_repmgr_ssl $clientdir
	setup_repmgr_ssl $clientdir2
	setup_repmgr_ssl $clientdir3

	#
	# Give each site a long election timeout so that there is time to
	# attempt to change priority during the election.
	#
	puts "\tRepmgr$tnum.ie.a: Start a master and three clients."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -timeout [list election 10000000] \
	    -start master

	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv repmgr -ack all -pri 50 \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -timeout [list election 10000000] \
	    -start client
	await_startup_done $clientenv

	set cl2_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT2 -home $clientdir2 -txn -rep -thread"
	set clientenv2 [eval $cl2_envcmd]
	$clientenv2 repmgr -ack all -pri 30 \
	    -local [list $hoststr [lindex $ports 2]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -timeout [list election 10000000] \
	    -start client
	await_startup_done $clientenv2

	set cl3_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT3 -home $clientdir3 -txn -rep -thread"
	set clientenv3 [eval $cl3_envcmd]
	$clientenv3 repmgr -ack all -pri 20 \
	    -local [list $hoststr [lindex $ports 3]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -timeout [list election 10000000] \
	    -start client
	await_startup_done $clientenv3

	puts "\tRepmgr$tnum.ie.b: Run some transactions at master."
	set start 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.ie.c: Shut down one client and master to cause\
	    unsuccessful election."
	#
	# We start an unsuccessful election to be sure it will still be
	# in progress when we try to change priority in the next step.
	#
	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0

	puts "\tRepmgr$tnum.ie.d: Attempt to change priority during election."
	# Pause to make sure election is started, then try to change priority.
	tclsleep 2
	catch {$clientenv2 repmgr -pri 100} res
	error_check_good errchk [is_substr $res "election in progress"] 1

	error_check_good client3_close [$clientenv3 close] 0
	error_check_good client2_close [$clientenv2 close] 0
}
