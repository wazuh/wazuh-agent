# Copyright (c) 2007, 2020 Oracle and/or its affiliates.  All rights reserved.
#
# See the file LICENSE for license information.
#
# $Id$
#
# TEST	repmgr013
# TEST	Site list test. 
# TEST
# TEST	Configure a master and two clients where one client is a peer of 
# TEST	the other and verify resulting site lists.
# TEST
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr013 { { niter 100 } { tnum "013" } args } {

	source ./include.tcl

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	set method "btree"
	set args [convert_args $method $args]

	puts "Repmgr$tnum ($method): repmgr site list test."
	repmgr013_sub $method $niter $tnum $args
}

proc repmgr013_sub { method niter tnum largs } {
	global testdir
	global rep_verbose
	global verbose_type
	global ipversion
	set nsites 4

	set small_iter [expr $niter / 10]

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	env_cleanup $testdir
	set ports [available_ports $nsites]
	set hoststr [get_hoststr $ipversion]

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

	puts "\tRepmgr$tnum.a: Start a master."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master

	puts "\tRepmgr$tnum.b: Start first client."
	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -remote [list $hoststr [lindex $ports 2]] \
	    -start client
	await_startup_done $clientenv

	puts "\tRepmgr$tnum.c: Start second client as peer of first."
	set cl2_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT2 -home $clientdir2 -txn -rep -thread"
	set clientenv2 [eval $cl2_envcmd]
	$clientenv2 repmgr -ack all \
	    -local [list $hoststr [lindex $ports 2]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -remote [list $hoststr [lindex $ports 1] peer] \
	    -start client
	await_startup_done $clientenv2

	puts "\tRepmgr$tnum.d: Start third unelectable client."
	set cl3_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT3 -home $clientdir3 -txn -rep -thread"
	set clientenv3 [eval $cl3_envcmd]
	$clientenv3 repmgr -ack all -pri 0 \
	    -local [list $hoststr [lindex $ports 3]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	await_startup_done $clientenv3

	puts "\tRepmgr$tnum.e: Verify repmgr site lists."
	verify_sitelist $masterenv $nsites {} [list [lindex $ports 3]] \
	    [list [lindex $ports 1] [lindex $ports 2]]
	verify_sitelist $clientenv $nsites {} [list [lindex $ports 3]] {}
	verify_sitelist $clientenv2 $nsites \
	    [list [lindex $ports 1]] [list [lindex $ports 3]] {}
	verify_sitelist $clientenv3 $nsites {} {} {}

	error_check_good client3_close [$clientenv3 close] 0
	error_check_good client2_close [$clientenv2 close] 0
	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}

#
# For numsites, supply the nsites value defined for the test.
# For peervec, supply a list of ports whose sites should be considered peers.
# For unelectvec, supply a list of ports for unelectable sites.
# For maxackvec on master site, supply a list of ports for sites sending acks.
#
proc verify_sitelist { env numsites peervec unelectvec maxackvec } {
	global ipversion
	set hoststr [get_hoststr $ipversion]
	set sitelist [$env repmgr_site_list]

	# Make sure there are expected number of other sites.
	error_check_good lenchk [llength $sitelist] [expr {$numsites - 1}]

	# Make sure eid and port are integers; host, status and peer are
	# the expected string values.
	set pvind 0
	foreach tuple $sitelist {
		error_check_good eidchk [string is integer -strict \
					     [lindex $tuple 0]] 1
		error_check_good hostchk [lindex $tuple 1] "$hoststr"
		set port [lindex $tuple 2]
		error_check_good portchk [string is integer -strict $port] 1
		error_check_good statchk [lindex $tuple 3] connected
		if { [lsearch $peervec $port] >= 0 } {
			error_check_good peerchk [lindex $tuple 4] peer
		} else {
			error_check_good npeerchk [lindex $tuple 4] non-peer
		}
		if { [lsearch $unelectvec $port] >= 0 } {
			error_check_good nunelchk \
			    [lindex $tuple 6] non-electable
		} else {
			error_check_good unelchk [lindex $tuple 6] electable
		}
		# Master site gets acks from electable other sites.
		if { [lsearch $maxackvec $port] >= 0 } {
			error_check_good mafchk \
			    [string is integer -strict [lindex $tuple 7]] 1
			error_check_good maochk \
			    [string is integer -strict [lindex $tuple 8]] 1
		# Master site gets no ack from unelectable other site.  Cannot
		# check all clients for [0,0] acks because they broadcast
		# their first ack to all sites because it is a new file number.
		# But we can check a master's unelectable site for [0,0].
		} elseif {[llength $maxackvec] > 0} {
			error_check_good zmafchk [lindex $tuple 7] 0
			error_check_good zmaochk [lindex $tuple 8] 0
		}
		incr pvind
	}
}
