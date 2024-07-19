#
# Copyright (c) 2017, 2020 Oracle and/or its affiliates.  All rights reserved.
#
# See the file LICENSE for license information.
#
# $Id$
#
# TEST	repmgr048
# TEST	Repmgr Poll/Epoll/Select Test.
# TEST
# TEST	Test if
# TEST	a) EPOLL, POLL and SELECT work as expected.
# TEST
# TEST	Poll, epoll and select all do the same thing.(with difference
# TEST	in implementation and performance). So to test if they are
# TEST	working we create a group with three sites, with each site
# TEST	employing a different method. Then we verify that we get to the
# TEST 	same expected end-state (which would have been reached had only
# TEST	select() been employed on all sites).
# TEST
# TEST	b) Flags for ENABLE_EPOLL and DISABLE_EPOLL work as expected.
# TEST
proc repmgr048 { { tnum "048" } args } {

	source ./include.tcl

	if { !$is_linux_test == 1 && !$is_osx_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	puts "Repmgr$tnum:Repmgr test for EPOLL, POLL and SELECT"

	repmgr048_sub $tnum 1000 0
	repmgr048_sub $tnum 1000 1
}

# Derived from the basic_repmgr_test of repmgr001
proc repmgr048_sub { tnum niter bulk } {

	source ./include.tcl
	global rep_verbose
	global verbose_type
	global overflowword1
	global overflowword2
	global ipversion
	set nsites 3
	set overflowword1 "0"
	set overflowword2 "0"
	set waitseconds 100

	set method "btree"

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	env_cleanup $testdir
	set ports [available_ports $nsites]
	set hoststr [get_hoststr $ipversion]

	set masterdir $testdir/MASTERDIR
	set clientdirs {}
	set clientenvs {}

	for { set i 1 } { $i < $nsites } { incr i} {
		set clientdir $testdir/CLIENTDIR$i
		file mkdir $clientdir

		setup_repmgr_ssl $clientdir

		lappend clientdirs $clientdir
	}

	file mkdir $masterdir

	setup_repmgr_ssl $masterdir

	set logtype "on-disk"
	set logargs [adjust_logargs $logtype]
	set txnargs [adjust_txnargs $logtype]

	set repmemarg ""

	set private ""

	# Open a master.
	puts "\tRepmgr$tnum.Poll.Epoll.test.a: Start an appointed master."
	set ma_envcmd "berkdb_env_noerr -create $logargs $verbargs \
	    $private \
	    -errpfx MASTER -home $masterdir $txnargs -rep -thread \
	    -lock_max_locks 10000 -lock_max_objects 10000 $repmemarg"
	set masterenv [eval $ma_envcmd]

	if { $is_linux_test == 1 } {
		#force EPOLL on.
		$masterenv rep_config {mgrenableepoll on}
	}
	$masterenv repmgr -ack all -pri 1000\
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master

	tclsleep 3

	if { $is_linux_test == 1 } {
		set poll_method_id [stat_field $masterenv repmgr_stat \
		    "Replication Manager Polling method"]
		#Verify that repmgr is using EPOLL().
		error_check_good poll_method_id $poll_method_id 3
		puts "\t\tRepmgr$tnum:Verified that master is using EPOLL."
	}

	# Start the clients.
	for {set i 1} {$i < $nsites} {incr i} {
		set v [expr "$i - 1"]
		set clientdir [lindex $clientdirs $v]
		puts "\tRepmgr$tnum.Poll.Epoll.Test.b.Client.$i: Start client. "

		set cl_envcmd "berkdb_env_noerr -create $verbargs $logargs \
		    $private \
		    -errpfx CLIENT$i -home $clientdir $txnargs -rep -thread \
		    -lock_max_locks 10000 -lock_max_objects 10000 $repmemarg"

		set clientenv [eval $cl_envcmd]
		if { $i == 1 } {
			if { $is_linux_test == 1 || $is_osx_test == 1 } {
				#Force use of select().
				$clientenv rep_config {mgrdisablepoll on}
			}
		}

		$clientenv repmgr -ack all -pri 0\
		    -local [list $hoststr [lindex $ports $i]] \
		    -remote [list $hoststr [lindex $ports 0]] \
		    -start client

		tclsleep 3

		if { $is_linux_test == 1 || $is_osx_test == 1 } {
			if { $i == 1} {
				set poll_method_id [stat_field $clientenv \
				    repmgr_stat \
				    "Replication Manager Polling method"]
				#Verify that repmgr is using SELECT().
				error_check_good poll_method_id \
				    $poll_method_id 1
				puts "\t\tRepmgr$tnum:Verified that this \
				    client is using SELECT."
			} else {
				set poll_method_id [stat_field $clientenv \
				    repmgr_stat \
				    "Replication Manager Polling method"]
				#Verify that repmgr is using POLL().
				error_check_good poll_method_id \
				    $poll_method_id 2
				puts "\t\tRepmgr$tnum:Verified that this \
				    client is using POLL."
			}
		}

		lappend clientenvs $clientenv
		await_startup_done $clientenv $waitseconds
	}

	puts "\tRepmgr$tnum.Poll.Epoll.Test.d: Run some transactions at master."
	if { $bulk } {
		# Turn on bulk processing on master.
		error_check_good set_bulk [$masterenv rep_config {bulk on}] 0

		eval rep_test_bulk $method $masterenv NULL $niter 0 0 0

		# Must turn off bulk because some configs (debug_rop/wop)
		# generate log records when verifying databases.
		error_check_good set_bulk [$masterenv rep_config {bulk off}] 0
	} else {
		eval rep_test $method $masterenv NULL $niter 0 0 0
	}

	puts "\tRepmgr$tnum.Poll.Epoll.Test.e: Verifying client \
	    database contents."
	for {set i 1} {$i < $nsites} {incr i} {
		set v [expr "$i - 1"]
		set clientdir [lindex $clientdirs $v]
		set clientenv [lindex $clientenvs $v]
		rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1
	}

	for {set i 1} {$i < $nsites} {incr i} {
		set v [expr "$i - 1"]
		set clientenv [lindex $clientenvs $v]
		error_check_good client_close [$clientenv close] 0
	}
	error_check_good masterenv_close [$masterenv close] 0
}
