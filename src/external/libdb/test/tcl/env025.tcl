# Copyright (c) 2013, 2020 Oracle and/or its affiliates. All rights reserved.
#
# See the file LICENSE for license information.
#
# $Id$
#
# TEST	env025
# TEST	Test db_recover with all allowed option combinations.
proc env025 { } {
	source ./include.tcl
	global has_crypto

	set encrypt 0
	if { $has_crypto == 1 } {
		lappend encrypt 1
	}

	# Test with -P -c -e -f -t -v and -b.
	foreach e $encrypt {
		foreach catastrophic { 1 0 } {
			foreach retain_env { 1 0 } {
				foreach show_percent { 1 0 } {
					foreach use_timestamp { 1 0 } {
						foreach verbose { 1 0 } {
							foreach  blob_dir \
							    { 1 0 } {
								env025_subtest \
								    $e \
								    $catastrophic \
								    $retain_env \
								    $show_percent \
								    $use_timestamp \
								    $verbose\
								    $blob_dir
							}
							
						}
					}
				}
			}
		}
	}

	set binname db_recover
	set std_redirect "> /dev/null"
	if { $is_windows_test } {
		set std_redirect "> /nul"
		append binname ".exe"
	}

	# Print version.
 	puts "\tEnv025: $util_path/$binname -V $std_redirect"
	set ret [catch {eval exec $util_path/$binname -V $std_redirect} r]
	error_check_good db_recover($r) $ret 0
}

proc env025_subtest { encrypt catastrophic retain_env show_percent \
    use_timestamp verbose blob_dir } {
	source ./include.tcl

	set passwd "passwd"
	set envargs ""
	set recover_args "-h $testdir"
	
	if { $catastrophic } {
		append recover_args " -c"
	}
	if { $retain_env } {
		append recover_args " -e"
	}
	if { $show_percent } {
		append recover_args " -f"
	}
	if { $verbose } {
		append recover_args " -v"
	}
	if { $encrypt } {
		append recover_args " -P $passwd"
		append envargs " -encryptaes $passwd"
	}

	set binname db_recover
	set std_redirect "> /dev/null"
	if { $is_windows_test } {
		set std_redirect "> /nul"
		append binname ".exe"
	}

	foreach region_dir { 1 0 } {
		
		puts "Env025: Test with options: (encrypt:$encrypt\
		    catastrophic:$catastrophic\
		    retain_env:$retain_env\
		    show_percent:$show_percent\
		    use_timestamp:$use_timestamp\
		    verbose:$verbose\
		    blob_dir:$blob_dir\
		    region_dir:$region_dir)"

		set key 1
		set data 1
		set skdata 2

		set method "-btree"
		set dbfile "env025.db"

		env_cleanup $testdir

		set recover_temp_args $recover_args
		set env_temp_args $envargs

		if { $blob_dir } {
			set blob_directory "__db_bl"
			file mkdir "$testdir/$blob_directory"

			append recover_temp_args " -b $blob_directory"
			append env_temp_args " -blob_dir $blob_directory"
		}

		# If the retain_env is not set, the environment will be set 
		# as private. The region_dir should not be set with the flag
		# DB_PRIVATE or DB_SYSTEM_MEM.
		if { $retain_env && $region_dir } {
			set regdir "REGDIR"
			file mkdir "$testdir/$regdir"

			append recover_temp_args " -r $regdir"
			append env_temp_args " -region_dir $regdir"
		}

		set env [eval berkdb_env \
		    $env_temp_args -create -txn -home $testdir]
		error_check_good env [is_valid_env $env] TRUE

		set db [eval {berkdb_open -env $env -create $method\
		    -mode 0644 -auto_commit $dbfile}]
		error_check_good dbopen [is_valid_db $db] TRUE

		# Initialize checkpoint.
		error_check_good "Initialize Checkpoint" [$env txn_checkpoint] 0

		set txn [$env txn]
		error_check_good txn_begin [is_valid_txn $txn $env] TRUE
		error_check_good db_put [$db put -txn $txn $key $data] 0
		error_check_good txn_commit [$txn commit] 0

		# We need to sleep before taking the timestamp to guarantee 
		# that the timestamp is *after* this transaction commits.
		tclsleep 1

		set timestamp [clock format [clock seconds] \
		    -format %Y%m%d%H%M.%S]

		# Sleep again to ensure that the next insert operation
		# definitely occurs after the timestamp.
		tclsleep 1

		set txn [$env txn]
		error_check_good txn_begin [is_valid_txn $txn $env] TRUE
		error_check_good db_put [$db put -txn $txn $key $skdata] 0
		error_check_good txn_commit [$txn commit] 0

		# We need to sleep before proceeding checkpoint to guarantee 
		# that the checkpoint is * after * this transaction commits.
		tclsleep 1
		error_check_good checkpoint [$env txn_checkpoint] 0

		if { $use_timestamp } {
			append recover_temp_args " -t $timestamp"
		}

		error_check_good db_close [$db close] 0
		error_check_good env_close [$env close] 0

  		puts "\tEnv025: $util_path/$binname \
		    $recover_temp_args $std_redirect"
		set ret [catch {eval exec \
		    $util_path/$binname $recover_temp_args $std_redirect} r]
		error_check_good db_recover($r) $ret 0

		if { $use_timestamp } {
			set db [eval berkdb_open $method $testdir/$dbfile]
			set dbt [$db get $key]
			set data [lindex [lindex $dbt 0] 1]

			error_check_good check_recover $data 1
			error_check_good db_close [$db close] 0
		}
		if { $retain_env } {
			# The environment should be retained.
			set env [eval berkdb_env_noerr \
			    -home $testdir $env_temp_args]
			error_check_good env_close [$env close] 0
		}
	}
}
