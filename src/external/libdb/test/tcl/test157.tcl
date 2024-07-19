# Copyright (c) 2017, 2020 Oracle and/or its affiliates.  All rights reserved.
#
# See the file LICENSE for license information.
#
# $Id$
#
# TEST	test157
# TEST	Test the utility db_upgrade with all combinations of flags
# TEST	except the flag -s.
proc test157 { method {nentries 100} {tnum "157"} args } {
	source ./include.tcl
	global passwd
	global EXE

	# Skip other access methods except btree.
	if { ![is_btree $method] } {
		puts "Test$tnum: skip for method $method."
		return
	}

	# Initialize database args.
	set dbargs ""
	set dbfile ""
	# Initialize flag args used in utility db_upgrade.
	set upgdargs ""
	
	set args [convert_args $method $args]
	set omethod [convert_method $method]
	
	# Extract the environment args.
	set eindex [lsearch -exact $args "-env"]
	set txnenv 0
	if { $eindex == -1 } {
		set env NULL
		set homestr $testdir
		set dbfile $homestr/test$tnum.db
	} else {
		incr eindex
		set env [lindex $args $eindex]
		set secenv [is_secenv $env]
		set txnenv [is_txnenv $env]
		set homestr [get_home $env]

		if { $secenv } {
			append upgdargs " -P $passwd "
		}
		if { $txnenv } {
			append dbargs " -auto_commit "
		}
		set dbfile test$tnum.db
	}
	
	set pwdindex [lsearch -exact $args "-encryptaes"]
	if { $pwdindex != -1 } {
		append upgdargs " -P $passwd "
	}

	# Clean up the environment.
	cleanup $homestr $env

	# Create a database.
	set db [eval {berkdb_open \
	     -create -mode 0644} $dbargs $args $omethod $dbfile]
	error_check_good db_open [is_valid_db $db] TRUE

	set did [open $dict]
	set count 0
	while { [gets $did str] != -1 && $count < $nentries } {
		# Insert key and data pairs into database.
		if { [is_record_based $method] } {
			set key [expr $count + 1]
		} else {
			set key $str
		}

		set rstr [reverse $str]
		set datastr $str:$rstr

		set data($key) [eval chop_data $method $datastr]
		error_check_good db_put \
			    [$db put $key $data($key)] 0

		incr count
	}
	close $did
	error_check_good db_close [$db close] 0

	puts "Test$tnum: $method ($args) testing db_upgrade."
	set binname db_upgrade
	set std_redirect "> /dev/null"
	if { $is_windows_test } {
		set std_redirect "> /nul"
		append binname $EXE
	}
	set msgpfx test$tnum
	set flags [list " -N " " -m $msgpfx " " -V "]
	if { $eindex == -1 || ![is_repenv $env] } {
		lappend flags " -S o " " -S v "
	}
	foreach flag $flags {
		foreach  verbose { 0 1 } {
			# Initialize the temp variable of upgdargs.
			set tupgdargs $upgdargs
			append tupgdargs $flag

			if { $eindex != -1 } {
				append tupgdargs " -h $homestr "
			}

			if { $verbose } {
				append tupgdargs " -v "
			}
			append tupgdargs $dbfile
			test157_execmd "$binname $tupgdargs "
		}
	}
}
proc test157_execmd { execmd } {
	source ./include.tcl
	global util_path

	puts "\tTest157:$util_path/$execmd"
	set result ""
	if { [catch {eval exec $util_path/$execmd} result] } {
		puts "FAIL: got $result while executing '$util_path/$execmd'"
	}
}
