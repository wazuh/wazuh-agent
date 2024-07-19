# Copyright (c) 2017, 2020 Oracle and/or its affiliates.  All rights reserved.
#
# See the file LICENSE for license information.
#
# $Id$
#
# TEST	env031
# TEST	Test utility db_tuner.
# TEST	
# TEST	Tests db_tuner with different situations about with/without off-page 
# TEST	duplicate page and with/without overflow page.  
proc env031 { } {
	source ./include.tcl

	set duplicate { 0 1 }
	set overflow { 0 1 }
	# Initialize the list of pagesize.
	set pgszlist { 512 1024 2048 4096 8192 16384 32768 65536 }

	puts "Env031: Test the utility db_tuner."
	foreach pgsize $pgszlist {
		foreach dupg $duplicate {
			foreach ovflpg $overflow {
				env031_body \
				    $pgsize \
				    $dupg \
				    $ovflpg
			}
		}
	}
}

proc env031_body { { pagesize 512 } { duplicate 0 } { overflow 0 } } {
	source ./include.tcl
	global EXE

	env_cleanup $testdir

	# Initialize tuner arguments.
	set tunargs "-h $testdir"
	# Initialize stat arguments.
	set statargs "-h $testdir"
	# Initialize environment arguments.
	set envargs "-create -home $testdir -txn"

	# Set 50MB for database cache.
	append envargs " -cachesize {0 50000000 0}"
	append tunargs " -c {0 50000000 0}"

	# Create environment with related argument.
	set env [eval {berkdb_env} $envargs]
	error_check_good db_env [is_valid_env $env] TRUE

	# Initialize the number of duplicate items.
	set ndups 1
	# Initialize the default length of overflow items.
	set ovflsize 4
	if { $overflow } {
		set ovflsize $pagesize
	}
	
	set dbname "env031"
	set dbfile "env031.db"
	# Initialize database arguments.
	set dbargs "-create -env $env \
	    -btree -auto_commit -pagesize $pagesize"

	append tunargs " -d $dbfile -s $dbname"
	if { $duplicate } {
		set ndups [expr $pagesize/50]
		append dbargs " -dup -dupsort"
	}
	append dbargs " $dbfile $dbname"

	# Create database with related argument.
	set db [eval {berkdb_open} $dbargs]
	error_check_good db_open [is_valid_db $db] TRUE

	set count 0
	# Open the file wordlist.
	set did [open $dict]

	# Put items into database.
	while { [gets $did str] != -1 && $count < 20 } {
		# Initialize random string generator.
		randstring_init $ndups

		if { $duplicate } {
			set prefix ""
			for { set i 1 } { $i <= $ndups } { incr i } {
				# Set a random string for prefix.
				set prefix [randstring]
			
				# Specify the data item.
				set datastr $prefix:$str
				error_check_good db_put \
				    [$db put $str $datastr] 0
			}
		}

		if { $overflow } {
			set prefix ""
			# Pad the data to satisfy the size of overflow
			# data items and the data will put into overflow page.
			set ovfldatasize [expr $ovflsize / 4 - 1]
			for { set j 1 } { $j <= $ovfldatasize } { incr j } {
				append prefix "!@#$"
			}
			# Specify the data item.
			set datastr $prefix:$str
			error_check_good db_put \
			    [$db put $str $datastr] 0
		}
		incr count
	}
	error_check_good db_sync [$db sync] 0

	# Check whether duplicate and overflow page are
	# created successfully.
	set ovflpgnum [stat_field $db stat "Overflow pages"]
	set dupgnum [stat_field $db stat "Duplicate pages"]

	error_check_good overflows [expr $ovflpgnum > 0] $overflow
	error_check_good duplicates [expr $dupgnum > 0] $duplicate

	close $did
	
	# Close database and environment handle.
	error_check_good db_close [$db close] 0
	error_check_good env_close [$env close] 0

	# 
	# db_tuner:
	# -c cachesize: Spefify a value of cachesize.
	# -d file: Display database statistics for the specified file.
	# -h home: Specify a home directory for the database environment.
	# -s database: Display page size recommendation for the specified
	# database.
	# -S o: Verify database with flag DB_NOORDERCHK specified.
	# -S v: Verify database with flag 0 specified.
	# -v verbose: Display verbose information.
	#
	set binname db_tuner
	set std_redirect "> /dev/null"
	if { $is_windows_test } {
		set std_redirect "> /nul"
		append binname $EXE
	}
	# Test the utility db_tuner.
	foreach verbose { 0 1 } {
		if { $verbose == 1 } {
			append tunargs " -v"
		}
		puts "\tEnv031: pagesize: $pagesize, \
		    duplicate: $duplicate, \
		    overflow: $overflow, \
		    verbose: $verbose."
		puts "\t\tEnv031.a:\
		    Verify database (skip btree) before performing db_tuner."
		env031_execmd "$binname $tunargs -S o $std_redirect"
		
		puts "\t\tEnv031.b:\
		    Verify database before performing db_tuner."
		env031_execmd "$binname $tunargs -S v $std_redirect"
	}
}
proc env031_execmd { execmd } {
	source ./include.tcl
	puts "\t\t\tEnv031:$util_path/$execmd"
	if { [catch {eval exec $util_path/$execmd} result] } {
		puts "FAIL: got $result while executing '$execmd'"
	}
}
