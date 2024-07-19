# Copyright (c) 2009, 2020 Oracle and/or its affiliates.  All rights reserved.
#
# See the file LICENSE for license information.
#
# TEST repmgr108
# TEST Subordinate connections and processes should not trigger elections.

proc repmgr108 { } {
	source ./include.tcl
	global ipversion

	set tnum "108"
	puts "Repmgr$tnum: Subordinate\
	    connections should not trigger elections."

	env_cleanup $testdir
	set hoststr [get_hoststr $ipversion]
	foreach {mport cport} [available_ports 2] {}
	file mkdir [set mdir $testdir/MASTER]
	file mkdir [set cdir $testdir/CLIENT]

	make_dbconfig $mdir \
	    [list [list repmgr_site $hoststr $mport db_local_site on]]
	setup_repmgr_ssl $mdir

	make_dbconfig $cdir \
	    [list [list repmgr_site $hoststr $cport db_local_site on] \
	    [list repmgr_site $hoststr $mport db_bootstrap_helper on]]
	setup_repmgr_ssl $cdir

	puts "\tRepmgr$tnum.a: Set up a pair of sites, two processes on master."
	set cmds {
		"home $mdir"
		"output $testdir/m1output"
		"open_env"
		"start master"
	}
	set m1 [open_site_prog [subst $cmds]]

	#
	# It is most common to start a subordinate process with the same start
	# value as the main replication process, but test here that we also
	# accept "start none", which supplies flags=0 to repmgr_start().
	#
	set cmds {
		"home $mdir"
		"output $testdir/m2output"
		"open_env"
		"start none"
	}
	set m2 [open_site_prog [subst $cmds]]

	set cmds {
		"home $cdir"
		"output $testdir/c1output"
		"open_env"
		"start client"
	}
	set c1 [open_site_prog [subst $cmds]]

	set cenv [berkdb_env -home $cdir]
	await_startup_done $cenv

	puts "\tRepmgr$tnum.b: Stop master's subordinate process (pause)."
	close $m2

	# Pause to let client notice the connection loss.
	tclsleep 3

	# We should see no elections ever having been started when master
	# subordinate process quits.
	# 
	set election_count [stat_field $cenv rep_stat "Elections held"]
	puts "\tRepmgr$tnum.c: Check election count ($election_count)."
	error_check_good no_elections $election_count 0

	$cenv close
	close $c1
	close $m1
}
