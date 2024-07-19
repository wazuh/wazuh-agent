# Copyright (c) 1996, 2020 Oracle and/or its affiliates.  All rights reserved.
#
# See the file LICENSE for license information.
#
# $Id$
#
# TEST	txn015
# TEST	Test the thread-switching problem with Go interface.[#26945]
# TEST	Start a multi-thread test to perform cursor operations, 
# TEST	transaction begin, transaction commit and transaction 
# TEST	abort.
proc txn015 { {tnum "015"} } {
	source ./include.tcl

	if [catch {package require Thread}] {
		puts "Skipping txn$tnum: requires Tcl Thread package."
		return 0
	}
	
	# Set up the user-defined error handling procedure.
	# Write the error message into errors.txt.
	global errorFile
	set errorFile [open errors.txt a]
	thread::errorproc logError

	set txn_commit "a"
	# Test thread-switching issue with transaction commit operation.
	txn015_runtxns $tnum $txn_commit

	set txn_abort "b"
	# Test thread-switching issue with transaction abort operation.
	txn015_runtxns $tnum $txn_abort

	# Close the file.
	close $errorFile
}

# Define txn main procedure.
proc txn015_runtxns { tnum txn_type } {
	source ./include.tcl
	if [catch {package require Thread}] {
		puts "Skipping txn$tnum: requires Tcl Thread package."
		return 0
	}

	env_cleanup $testdir

	if {$txn_type == "a"} {
		puts "Txn$tnum.$txn_type: Test the\
		    thread-switching issue with transaction commit operation."
	} elseif {$txn_type == "b"} {
		puts "Txn$tnum.$txn_type: Test the\
		    thread-switching issue with transaction abort operation."
	}
	puts "\tTxn$tnum.$txn_type.0: Initialize\
	    environment and database in main thread."
	set env_init_ret [berkdb thread_env_init $testdir]
	error_check_good env_init $env_init_ret 0

	puts "\tTxn$tnum.$txn_type.1: Transaction begin in new thread."
	thread_txn_begin

	puts "\tTxn$tnum.$txn_type.2: Create cursor handle in new thread."
	thread_dbc_create

	puts "\tTxn$tnum.$txn_type.3: Cursor put."
	# Do cursor put in normal way.
	set key 1
	set data 1
	set ret [berkdb thread_dbc_put $key $data]

	puts "\tTxn$tnum.$txn_type.4: Cursor put in new thread."
	set key 2
	set data 2
	# Obtain the thread id.
	set dbc_put_threadID [thread_dbc_put]
	thread::send -async $dbc_put_threadID [list dbc_put $key $data] result

	puts "\tTxn$tnum.$txn_type.5: Cursor get in new thread."
	set key 1
	# Obtain the thread id.
	set dbc_get_threadID [thread_dbc_get]
	thread::send -async $dbc_get_threadID [list dbc_get $key] result

	# Wait the value returned as result which 
	# presents the termination of the thread.
	for {set i 0} {$i < 2} {incr i} {
		vwait result
	}

	puts "\tTxn$tnum.$txn_type.6: Cursor close in new thread."
	thread_dbc_close

	if {$txn_type == "a"} {
		puts "\tTxn$tnum.$txn_type.7:\
		    Transaction commit in new thread."
		thread_txn_commit

		puts "\tTxn$tnum.$txn_type.8:\
		    Verify data is in database."
		set txn_begin_ret [berkdb thread_txn_begin]
		error_check_good thread_txn $txn_begin_ret 0

		set dbc_create_ret [berkdb thread_dbc_create]
		error_check_good dbc_create $dbc_create_ret 0

		set key 2
		set data 2
		set dbc_get_ret [berkdb thread_dbc_get $key]
		error_check_bad dbc_get_length [llength $dbc_get_ret] 0
		error_check_good dbc_get_data \
		    [lindex [lindex $dbc_get_ret 0] 1] $data

		set dbc_close_ret [berkdb thread_dbc_close]
		error_check_good dbc_close $dbc_close_ret 0

		set txn_commit_ret [berkdb thread_txn_commit]
		error_check_good txn_commit $txn_commit_ret 0

	} elseif {$txn_type == "b"} {
		puts "\tTxn$tnum.$txn_type.7: Transaction abort in new thread."
		thread_txn_abort

		puts "\tTxn$tnum.$txn_type.8: Verify data is not in database."
		set txn_begin_ret [berkdb thread_txn_begin]
		error_check_good thread_txn $txn_begin_ret 0

		set dbc_create_ret [berkdb thread_dbc_create]
		error_check_good dbc_create $dbc_create_ret 0

		set key 2
		set data 2
		set dbc_get_result [berkdb thread_dbc_get $key]
		set dbc_get_expected \
		    "BDB0073 DB_NOTFOUND: No matching key/data pair found"
		set dbc_get_ret \
		    [string compare $dbc_get_result $dbc_get_expected]
		error_check_good dbc_get $dbc_get_ret 0

		set dbc_close_ret [berkdb thread_dbc_close]
		error_check_good dbc_close $dbc_close_ret 0

		set txn_commit_ret [berkdb thread_txn_commit]
		error_check_good txn_commit $txn_commit_ret 0
	}

	puts "\tTxn$tnum.$txn_type.8: Dispose environment in main thread."
	set env_dispose_ret [berkdb thread_env_dispose]
	error_check_good env_dispose $env_dispose_ret 0

	puts "\tTxn$tnum.$txn_type.9: Dispose all threads in main thread."
	# Obtain all active threads' id.
	set thread_id_list [thread::names]

	# Release all existent threads except the main thread.
	foreach thread_id $thread_id_list {
		thread::release $thread_id
	}
}

# Define txn begin operation in another thread.
proc thread_txn_begin { } {
	set txn_begin_threadID [thread::create -joinable {
		source ./include.tcl
		source $test_path/test.tcl

		# Begin transaction.
		set txn_begin_ret [berkdb thread_txn_begin]
		error_check_good txn_begin $txn_begin_ret 0
	}]
	# Wait the target thread to terminates.
	thread::join $txn_begin_threadID
}

# Define txn commit operation in another thread.
proc thread_txn_commit { } {
	set txn_commit_threadID [thread::create -joinable {
		source ./include.tcl
		source $test_path/test.tcl

		# Commit transaction.
		set txn_commit_ret [berkdb thread_txn_commit]
		error_check_good txn_commit $txn_commit_ret 0
	}]
	# Wait the target thread to terminates.
	thread::join $txn_commit_threadID
}

# Define txn abort operation in another thread.
proc thread_txn_abort { } {
	set txn_abort_threadID [thread::create -joinable {
		source ./include.tcl
		source $test_path/test.tcl

		# Abort transaction.
		set txn_abort_ret [berkdb thread_txn_abort]
		error_check_good txn_abort $txn_abort_ret 0
	}]
	# Wait the target thread to terminates.
	thread::join $txn_abort_threadID
}

# Define dbc create operation in another thread.
proc thread_dbc_create { } {
	set dbc_create_threadID [thread::create -joinable {
		source ./include.tcl
		source $test_path/test.tcl

		# Create cursor handle.
		set dbc_create_ret [berkdb thread_dbc_create]
		error_check_good dbc_create $dbc_create_ret 0
	}]
	# Wait the target thread to terminates.
	thread::join $dbc_create_threadID
}

# Define dbc put operation in another thread.
proc thread_dbc_put { } {
	set dbc_put_threadID [thread::create -joinable {
		source ./include.tcl
		source $test_path/test.tcl

		# Cursor put key/data pair.
		proc dbc_put { key data } {
			set dbc_put_ret [berkdb thread_dbc_put $key $data]
			puts "\t\tTxn015: Cursor put $key $data"
			puts "\t\tTxn015: Cursor put return $dbc_put_ret"
			return $dbc_put_ret
		}
		thread::wait
	}]
	# Return the current thread id to the main process.
	return $dbc_put_threadID
}

# Define dbc get operation in another thread.
proc thread_dbc_get { } {
	set dbc_get_threadID [thread::create {
		source ./include.tcl
		source $test_path/test.tcl

		# Cursor get key/data pair.
		proc dbc_get { key } {
			set dbc_get_ret [berkdb thread_dbc_get $key]
			puts "\t\tTxn015: Cursor get $key"
			puts "\t\tTxn015: Cursor get return $dbc_get_ret"
		}
		thread::wait
	}]
	# Return the current thread id to the main process.
	return $dbc_get_threadID
}

# Define dbc close operation in another thread.
proc thread_dbc_close { } {
	set dbc_close_threadID [thread::create -joinable {
		source ./include.tcl
		source $test_path/test.tcl

		# Close cursor.
		set dbc_close_ret [berkdb thread_dbc_close]
		error_check_good dbc_close $dbc_close_ret 0
	}]
	# Wait the target thread to terminates.
	thread::join $dbc_close_threadID
}

# User-defined error handling procedure.
#	id 	id of thread 
#	error 	stack trace
proc logError { id error } {
	global errorFile

	puts $errorFile "Error in thread $id."
	puts $errorFile "\t$error."
	puts $errorFile ""
}
