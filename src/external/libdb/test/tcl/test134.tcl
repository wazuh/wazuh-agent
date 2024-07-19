# Copyright (c) 2011, 2020 Oracle and/or its affiliates.  All rights reserved.
#
# See the file LICENSE for license information.
#
# $Id$
#
# TEST	test134
# TEST  Test cursor cleanup for sub databases.

proc test134 {method {nentries 1000} args} {
	source ./include.tcl

	eval {test133 $method $nentries "134" 1} $args

}
