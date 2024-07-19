# $Id$

topdir=		..
srcdir=		$(topdir)/src
langdir=	$(topdir)/lang
exampledir=	$(topdir)/examples
testdir=	$(topdir)/test
utildir=	$(topdir)/util
distdir=	$(topdir)/dist
builddir=.

##################################################
# Installation directories and permissions.
##################################################
prefix=	/usr/local/BerkeleyDB.18.1
exec_prefix=${prefix}
bindir=	${exec_prefix}/bin
includedir=${prefix}/include
libdir=	${exec_prefix}/lib
docdir=	$(prefix)/docs

dmode=	755
emode=	555
fmode=	444

transform=s,x,x,

##################################################
# Paths for standard user-level commands.
##################################################
AR=	ar
CHMOD=	chmod
CP=	cp
LN=	ln
MKDIR=	mkdir
MV=	mv
RANLIB=	ranlib
RM=	rm -f
SHELL=	/bin/sh
STRIP=	strip
SED=	/bin/sed
PERL=	

##################################################
# General library information.
##################################################
DEF_LIB=	$(libso_target)
DEF_LIB_CXX=	
DEF_LIB_SQL=	@DEFAULT_LIB_SQL@
DEF_LIB_SQLITE=	@DEFAULT_LIB_SQLITE@
DEF_LIB_STL=	
DEF_LIB_TCL=	
INSTALLER=	$(LIBTOOL) --mode=install cp -p
LIBTOOL=	./libtool

POSTLINK=	$(LIBTOOL) --mode=execute true
SOLINK=		$(LIBTOOL) --mode=link cc -avoid-version -O3 
SOFLAGS=	-rpath $(libdir)
LIBMAJOR=	18
LIBVERSION=	18.1

CPPFLAGS=	-I$(builddir) -I$(srcdir) -fPIC -D_GNU_SOURCE -D_REENTRANT

##################################################
# C API.
##################################################
CFLAGS=		-c $(CPPFLAGS) -O3 
CC=		$(LIBTOOL) --mode=compile cc
CCLINK=		$(LIBTOOL) --mode=link cc -O3 

LDFLAGS=	
LIBS=		 -lpthread
TEST_LIBS=	 -lm -lnsl
LIBCSO_LIBS=	  -lpthread

libdb_base=	libdb
libdb=		$(libdb_base).a
libdb_version=	$(libdb_base)-$(LIBVERSION).a
libso=		$(libdb_base)-$(LIBVERSION).so
libso_target=	$(libdb_base)-$(LIBVERSION).la
libso_default=	$(libdb_base).so
libso_major=	$(libdb_base)-$(LIBMAJOR).so

##################################################
# C++ API.
#
# C++ support is optional, and can be built with static or shared libraries.
##################################################
CXXFLAGS=	-c $(CPPFLAGS) -O3 
CXX=		$(LIBTOOL) --mode=compile nocxx
CXXLINK=	$(LIBTOOL) --mode=link nocxx -O3 
XSOLINK=	$(LIBTOOL) --mode=link nocxx -avoid-version -O3 
LIBXSO_LIBS=	  -lpthread

libcxx_base=	libdb_cxx
libcxx=		$(libcxx_base).a
libcxx_version=	$(libcxx_base)-$(LIBVERSION).a
libxso=		$(libcxx_base)-$(LIBVERSION).so
libxso_target=	$(libcxx_base)-$(LIBVERSION).la
libxso_default=	$(libcxx_base).so
libxso_major=	$(libcxx_base)-$(LIBMAJOR).so

##################################################
# SQL API.
#
# SQL support is optional, and can be built with static or shared libraries.
##################################################
SQLFLAGS=	-I$(builddir) -I$(builddir)/sql -I$(langdir)/sql/generated \
		-D_HAVE_SQLITE_CONFIG_H @SQL_FLAGS@
LIBSQLSO_LIBS=     -lpthread

libsql_base=	libdb_sql
libsql=		$(libsql_base).a
libsql_version=	$(libsql_base)-$(LIBVERSION).a
libsqlso=	$(libsql_base)-$(LIBVERSION).so
libsqlso_target=$(libsql_base)-$(LIBVERSION).la
libsqlso_default=$(libsql_base).so
libsqlso_major=	$(libsql_base)-$(LIBMAJOR).so

libsqlite_base=	libsqlite3
libsqlite=	$(libsqlite_base).a
libsqliteso=	$(libsqlite_base).so
libsqliteso_target=$(libsqlite_base).la

##################################################
# STL API.
#
# STL support is optional, and can be built with static or shared libraries.
##################################################
STLFLAGS=	$(CXXFLAGS) -I$(langdir)/cxx/stl
LIBSTLSO_LIBS=	  -lpthread

libstl_base=	libdb_stl
libstl=		$(libstl_base).a
libstl_version=	$(libstl_base)-$(LIBVERSION).a
libstlso=	$(libstl_base)-$(LIBVERSION).so
libstlso_target=$(libstl_base)-$(LIBVERSION).la
libstlso_default=$(libstl_base).so
libstlso_major=	$(libstl_base)-$(LIBMAJOR).so

##################################################
# Java API and GUI.
#
# Java support is optional and requires shared librarires.
##################################################
GUITESTCLASSPATH=
CLASSPATH=	$(JAVA_CLASSTOP)
LIBJSO_LIBS=	  -lpthread
SWIGCFLAGS=	

JAR=		
JAVA=		env CLASSPATH="$(CLASSPATH)" 
JAVAC=		env CLASSPATH="$(CLASSPATH)" nojavac
JAVACFLAGS=	
JAVA_CLASSTOP=	./classes
JAVA_EXCLASSTOP=./classes.ex
JAVA_DEMOCLASSTOP=./classes.demo
JAVA_GUICLASSTOP=./classes.gui
JAVA_GUITESTCLASSTOP=./classes.guitest
JAVA_SRCDIR=	$(langdir)/java/src
JAVA_EXDIR=	$(exampledir)/java/src
JAVA_DEMODIR=	$(exampledir)/java/parking_demo/src/com/sleepycat/sample
JAVA_GUIDIR=	$(utildir)/db_gui
JAVA_GUITESTDIR=$(testdir)/db_guitest
JAVA_SLEEPYCAT=	$(langdir)/java/src/com/sleepycat
JAVA_MANIFEST=	$(langdir)/java/jarManifestEntries

JDK_VERSION=	`$(JAVAC) -version 2>&1 | sed 's/.*\.\(.*\)\..*/\\1/'`

libj_jarfile=	db.jar
libj_exjarfile=	dbexamples.jar
libj_demojarfile=dbdemos.jar
libj_guijarfile=db_gui.jar
libj_guitestjarfile=db_guitest.jar
libjso_base=	libdb_java
libjso=		$(libjso_base)-$(LIBVERSION).so
libjso_static=	$(libjso_base)-$(LIBVERSION).a
libjso_target=	$(libjso_base)-$(LIBVERSION).la
libjso_default=	$(libjso_base).so
libjso_major=	$(libjso_base)-$(LIBMAJOR).so
libjso_g=	$(libjso_base)-$(LIBVERSION)_g.so

##################################################
# TCL API.
#
# Tcl support is optional.
##################################################
TCL_INCLUDE_SPEC=	
LIBTSO_LIBS=	  -lpthread
TCL_SRCDIR=	$(langdir)/tcl
libtcl_base=	libdb_tcl
libtcl=		$(libtcl_base).a
libtcl_version=	$(libtcl_base)-$(LIBVERSION).a
libtso=		$(libtcl_base)-$(LIBVERSION).so
libtso_target=	$(libtcl_base)-$(LIBVERSION).la
libtso_default=	$(libtcl_base).so
libtso_major=	$(libtcl_base)-$(LIBMAJOR).so

##################################################
# db_dump185 UTILITY
#
# The db_dump185 application should be compiled using the system's db.h file
# (which should be a DB 1.85/1.86 include file), and the system's 1.85/1.86
# object library.  To include the right db.h, don't include -I$(builddir) on
# the compile line.  You may also need to add a local include directory and
# local libraries, for example.  Do that by adding -I options to the DB185INC
# line, and -l options to the DB185LIB line.
##################################################
DB185INC=	-c -O3  -I$(topdir) -fPIC -D_GNU_SOURCE -D_REENTRANT
DB185LIB=

##################################################
# Performance Event Monitoring definitions
##################################################
DTRACE=			
DTRACE_PROVIDER=	$(distdir)/db_provider.d

##################################################
# NOTHING BELOW THIS LINE SHOULD EVER NEED TO BE MODIFIED.
##################################################

##################################################
# Object and utility lists.
##################################################
BTREE_OBJS=\
	bt_compare.lo bt_compress.lo bt_conv.lo bt_curadj.lo bt_cursor.lo \
	bt_delete.lo bt_method.lo bt_open.lo bt_put.lo bt_rec.lo \
	bt_reclaim.lo bt_recno.lo bt_rsearch.lo bt_search.lo bt_split.lo bt_stat.lo \
	bt_compact.lo bt_upgrade.lo btree_auto.lo
BTREE_VRFY_OBJS=\
	db_ovfl_vrfy.lo db_vrfy.lo db_vrfyutil.lo bt_verify.lo
HASH_OBJS=\
	hash.lo hash_auto.lo hash_compact.lo hash_conv.lo hash_dup.lo \
	hash_meta.lo hash_method.lo hash_open.lo hash_page.lo hash_rec.lo \
	hash_reclaim.lo hash_stat.lo hash_upgrade.lo
HASH_VRFY_OBJS=\
	hash_verify.lo
HEAP_OBJS=\
	heap.lo heap_auto.lo heap_backup.lo heap_conv.lo heap_method.lo \
	heap_open.lo heap_rec.lo heap_reclaim.lo heap_stat.lo heap_upgrade.lo
HEAP_VRFY_OBJS=\
	heap_verify.lo
QUEUE_OBJS=\
	qam.lo qam_auto.lo qam_conv.lo qam_files.lo qam_method.lo \
	qam_open.lo qam_rec.lo qam_stat.lo qam_upgrade.lo
QUEUE_VRFY_OBJS=\
	qam_verify.lo
LOCK_OBJS=\
	lock.lo lock_deadlock.lo lock_failchk.lo lock_id.lo lock_list.lo \
	lock_method.lo lock_region.lo lock_stat.lo lock_timer.lo lock_util.lo

LOG_VRFY_OBJS=\
	log_verify.lo log_verify_util.lo log_verify_int.lo \
	log_verify_auto.lo 

MUTEX_OBJS=\
	mut_alloc.lo mut_failchk.lo mut_method.lo mut_region.lo mut_stat.lo
REP_OBJS=\
	rep_automsg.lo rep_backup.lo rep_elect.lo rep_lease.lo rep_log.lo \
	rep_method.lo rep_record.lo rep_region.lo rep_stat.lo \
	rep_util.lo rep_verify.lo
REPMGR_OBJS=\
	os_addrinfo.lo\
	repmgr_auto.lo repmgr_automsg.lo repmgr_elect.lo \
	repmgr_method.lo repmgr_msg.lo  \
	repmgr_net.lo repmgr_posix.lo repmgr_queue.lo repmgr_rec.lo \
	repmgr_sel.lo repmgr_stat.lo repmgr_util.lo
PRINT_OBJS=\
	btree_autop.lo crdel_autop.lo db_autop.lo dbreg_autop.lo \
	fileops_autop.lo hash_autop.lo heap_autop.lo qam_autop.lo \
	repmgr_autop.lo txn_autop.lo
XA_OBJS=\
	xa.lo xa_map.lo

# When DTrace is enabled it may need to post-process (with -G) most of the
# object files in order to generate the additional objects in .

DTRACE_OBJS=  $(BTREE_OBJS) $(HASH_OBJS) heap_stub.lo qam_stub.lo rep_stub.lo repmgr_stub.lo db_vrfy_stub.lo log_verify_stub.lo lock_stub.lo mut_stub.lo  crypto_stub.lo \
	blob_fileops.lo blob_page.lo blob_stream.lo blob_util.lo \
	clock.lo crdel_auto.lo crdel_rec.lo db.lo db_am.lo db_auto.lo \
	db_backup.lo db_byteorder.lo db_cam.lo db_cds.lo db_compact.lo \
	db_compint.lo db_conv.lo db_copy.lo db_dispatch.lo db_dup.lo \
	db_err.lo db_getlong.lo db_idspace.lo db_iface.lo db_join.lo \
	db_log2.lo db_meta.lo db_method.lo db_open.lo db_overflow.lo \
	db_pr.lo db_rec.lo db_reclaim.lo db_remove.lo db_rename.lo db_ret.lo \
	db_setid.lo db_setlsn.lo db_slice.lo db_shash.lo db_sort_multiple.lo \
	db_stati.lo db_truncate.lo db_upg.lo db_upg_opd.lo dbreg.lo \
	dbreg_stat.lo dbreg_auto.lo dbreg_rec.lo dbreg_util.lo \
	dbt.lo env_alloc.lo env_config.lo env_backup.lo env_failchk.lo \
	env_file.lo env_globals.lo env_open.lo env_method.lo env_name.lo \
	env_recover.lo env_region.lo env_register.lo env_sig.lo env_slice.lo \
	env_stat.lo fileops_auto.lo fop_basic.lo fop_rec.lo fop_util.lo \
	hash_func.lo hmac.lo log.lo log_archive.lo log_compare.lo \
	log_debug.lo log_get.lo log_method.lo log_print.lo log_put.lo \
	log_stat.lo mkpath.lo mp_alloc.lo mp_backup.lo mp_bh.lo mp_fget.lo \
	mp_fmethod.lo mp_fopen.lo mp_fput.lo mp_fset.lo mp_method.lo \
	mp_mvcc.lo mp_region.lo mp_register.lo mp_resize.lo mp_stat.lo \
	mp_sync.lo mp_trickle.lo openflags.lo os_abort.lo os_abs.lo \
	os_alloc.lo os_clock.lo os_cpu.lo os_ctime.lo os_config.lo \
	os_dir.lo os_errno.lo os_fid.lo os_flock.lo os_fsync.lo \
	os_getenv.lo os_handle.lo os_map.lo os_method.lo os_mkdir.lo \
	os_open.lo os_path.lo os_pid.lo os_rename.lo os_rmdir.lo os_root.lo \
	os_rpath.lo os_rw.lo os_seek.lo os_stack.lo os_stat.lo \
	os_tmpdir.lo os_truncate.lo os_uid.lo os_unlink.lo os_yield.lo \
	partition.lo seq_stat.lo sequence.lo sha1.lo snprintf.lo txn.lo \
	txn_auto.lo txn_chkpt.lo txn_failchk.lo txn_method.lo txn_rec.lo \
	txn_recover.lo txn_region.lo txn_stat.lo txn_util.lo xa.lo \
	xa_map.lo zerofill.lo \

C_OBJS=	$(DTRACE_OBJS) 

CUTEST_OBJS=\
	CuTest.lo CuTests.lo Runner.lo TestCallbackSetterAndGetter.lo \
	TestChannel.lo TestDbHotBackup.lo TestDbTuner.lo TestEncryption.lo \
	TestEnvConfig.lo TestEnvMethod.lo TestKeyExistErrorReturn.lo \
	TestMutexAlignment.lo TestPartial.lo TestPartition.lo \
	TestPreOpenSetterAndGetter.lo TestQueue.lo

CXX_OBJS=\
	cxx_channel.lo cxx_db.lo cxx_dbc.lo cxx_dbstream.lo cxx_dbt.lo \
	cxx_env.lo cxx_except.lo cxx_lock.lo cxx_logc.lo cxx_mpool.lo \
	cxx_multi.lo cxx_rid.lo cxx_seq.lo cxx_site.lo cxx_txn.lo

CRYPTO_OBJS=\
	aes_method.lo crypto.lo mt19937db.lo rijndael-alg-fst.lo \
	rijndael-api-fst.lo

JAVA_OBJS=\
	db_java_wrap.lo

JAVA_DBSRCS=\
	$(JAVA_SLEEPYCAT)/bind/ByteArrayBinding.java \
	$(JAVA_SLEEPYCAT)/bind/EntityBinding.java \
	$(JAVA_SLEEPYCAT)/bind/EntryBinding.java \
	$(JAVA_SLEEPYCAT)/bind/RecordNumberBinding.java \
	$(JAVA_SLEEPYCAT)/bind/serial/ClassCatalog.java \
	$(JAVA_SLEEPYCAT)/bind/serial/SerialBase.java \
	$(JAVA_SLEEPYCAT)/bind/serial/SerialBinding.java \
	$(JAVA_SLEEPYCAT)/bind/serial/SerialInput.java \
	$(JAVA_SLEEPYCAT)/bind/serial/SerialOutput.java \
	$(JAVA_SLEEPYCAT)/bind/serial/SerialSerialBinding.java \
	$(JAVA_SLEEPYCAT)/bind/serial/SerialSerialKeyCreator.java \
	$(JAVA_SLEEPYCAT)/bind/serial/StoredClassCatalog.java \
	$(JAVA_SLEEPYCAT)/bind/serial/TupleSerialBinding.java \
	$(JAVA_SLEEPYCAT)/bind/serial/TupleSerialKeyCreator.java \
	$(JAVA_SLEEPYCAT)/bind/serial/TupleSerialMarshalledBinding.java \
	$(JAVA_SLEEPYCAT)/bind/serial/TupleSerialMarshalledKeyCreator.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/BigDecimalBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/BigIntegerBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/BooleanBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/ByteBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/CharacterBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/DoubleBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/FloatBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/IntegerBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/LongBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/MarshalledTupleEntry.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/MarshalledTupleKeyEntity.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/PackedIntegerBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/PackedLongBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/ShortBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/SortedBigDecimalBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/SortedDoubleBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/SortedFloatBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/SortedPackedIntegerBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/SortedPackedLongBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/StringBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleBase.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleInput.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleInputBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleMarshalledBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleOutput.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleTupleBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleTupleKeyCreator.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleTupleMarshalledBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleTupleMarshalledKeyCreator.java \
	$(JAVA_SLEEPYCAT)/collections/BaseIterator.java \
	$(JAVA_SLEEPYCAT)/collections/BlockIterator.java \
	$(JAVA_SLEEPYCAT)/collections/CurrentTransaction.java \
	$(JAVA_SLEEPYCAT)/collections/DataCursor.java \
	$(JAVA_SLEEPYCAT)/collections/DataView.java \
	$(JAVA_SLEEPYCAT)/collections/MapEntryParameter.java \
	$(JAVA_SLEEPYCAT)/collections/MyRangeCursor.java \
	$(JAVA_SLEEPYCAT)/collections/PrimaryKeyAssigner.java \
	$(JAVA_SLEEPYCAT)/collections/StoredCollection.java \
	$(JAVA_SLEEPYCAT)/collections/StoredCollections.java \
	$(JAVA_SLEEPYCAT)/collections/StoredContainer.java \
	$(JAVA_SLEEPYCAT)/collections/StoredEntrySet.java \
	$(JAVA_SLEEPYCAT)/collections/StoredIterator.java \
	$(JAVA_SLEEPYCAT)/collections/StoredKeySet.java \
	$(JAVA_SLEEPYCAT)/collections/StoredList.java \
	$(JAVA_SLEEPYCAT)/collections/StoredMap.java \
	$(JAVA_SLEEPYCAT)/collections/StoredMapEntry.java \
	$(JAVA_SLEEPYCAT)/collections/StoredSortedEntrySet.java \
	$(JAVA_SLEEPYCAT)/collections/StoredSortedKeySet.java \
	$(JAVA_SLEEPYCAT)/collections/StoredSortedMap.java \
	$(JAVA_SLEEPYCAT)/collections/StoredSortedValueSet.java \
	$(JAVA_SLEEPYCAT)/collections/StoredValueSet.java \
	$(JAVA_SLEEPYCAT)/collections/TransactionRunner.java \
	$(JAVA_SLEEPYCAT)/collections/TransactionWorker.java \
	$(JAVA_SLEEPYCAT)/collections/TupleSerialFactory.java \
	$(JAVA_SLEEPYCAT)/compat/DbCompat.java \
	$(JAVA_SLEEPYCAT)/db/BackupHandler.java \
	$(JAVA_SLEEPYCAT)/db/BackupOptions.java \
	$(JAVA_SLEEPYCAT)/db/BtreeCompressor.java \
	$(JAVA_SLEEPYCAT)/db/BtreePrefixCalculator.java \
	$(JAVA_SLEEPYCAT)/db/BtreeStats.java \
	$(JAVA_SLEEPYCAT)/db/CacheFile.java \
	$(JAVA_SLEEPYCAT)/db/CacheFilePriority.java \
	$(JAVA_SLEEPYCAT)/db/CacheFileStats.java \
	$(JAVA_SLEEPYCAT)/db/CacheStats.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationChannel.java \
	$(JAVA_SLEEPYCAT)/db/CheckpointConfig.java \
	$(JAVA_SLEEPYCAT)/db/CompactConfig.java \
	$(JAVA_SLEEPYCAT)/db/CompactStats.java \
	$(JAVA_SLEEPYCAT)/db/Cursor.java \
	$(JAVA_SLEEPYCAT)/db/CursorConfig.java \
	$(JAVA_SLEEPYCAT)/db/Database.java \
	$(JAVA_SLEEPYCAT)/db/DatabaseConfig.java \
	$(JAVA_SLEEPYCAT)/db/DatabaseEntry.java \
	$(JAVA_SLEEPYCAT)/db/DatabaseException.java \
	$(JAVA_SLEEPYCAT)/db/DatabaseStats.java \
	$(JAVA_SLEEPYCAT)/db/DatabaseStream.java \
	$(JAVA_SLEEPYCAT)/db/DatabaseStreamConfig.java \
	$(JAVA_SLEEPYCAT)/db/DatabaseType.java \
	$(JAVA_SLEEPYCAT)/db/DeadlockException.java \
	$(JAVA_SLEEPYCAT)/db/Environment.java \
	$(JAVA_SLEEPYCAT)/db/EnvironmentConfig.java \
	$(JAVA_SLEEPYCAT)/db/ErrorHandler.java \
	$(JAVA_SLEEPYCAT)/db/EventHandler.java \
	$(JAVA_SLEEPYCAT)/db/EventHandlerAdapter.java \
	$(JAVA_SLEEPYCAT)/db/FeedbackHandler.java \
	$(JAVA_SLEEPYCAT)/db/ForeignKeyDeleteAction.java \
	$(JAVA_SLEEPYCAT)/db/ForeignKeyNullifier.java \
	$(JAVA_SLEEPYCAT)/db/ForeignMultiKeyNullifier.java \
	$(JAVA_SLEEPYCAT)/db/HashStats.java \
	$(JAVA_SLEEPYCAT)/db/Hasher.java \
	$(JAVA_SLEEPYCAT)/db/HeapRecordId.java \
	$(JAVA_SLEEPYCAT)/db/HeapFullException.java \
	$(JAVA_SLEEPYCAT)/db/HeapStats.java \
	$(JAVA_SLEEPYCAT)/db/JoinConfig.java \
	$(JAVA_SLEEPYCAT)/db/JoinCursor.java \
	$(JAVA_SLEEPYCAT)/db/KeyRange.java \
	$(JAVA_SLEEPYCAT)/db/Lock.java \
	$(JAVA_SLEEPYCAT)/db/LockDetectMode.java \
	$(JAVA_SLEEPYCAT)/db/LockMode.java \
	$(JAVA_SLEEPYCAT)/db/LockNotGrantedException.java \
	$(JAVA_SLEEPYCAT)/db/LockOperation.java \
	$(JAVA_SLEEPYCAT)/db/LockRequest.java \
	$(JAVA_SLEEPYCAT)/db/LockRequestMode.java \
	$(JAVA_SLEEPYCAT)/db/LockStats.java \
	$(JAVA_SLEEPYCAT)/db/LogCursor.java \
	$(JAVA_SLEEPYCAT)/db/LogRecordHandler.java \
	$(JAVA_SLEEPYCAT)/db/LogSequenceNumber.java \
	$(JAVA_SLEEPYCAT)/db/LogStats.java \
	$(JAVA_SLEEPYCAT)/db/LogVerifyConfig.java \
	$(JAVA_SLEEPYCAT)/db/MemoryException.java \
	$(JAVA_SLEEPYCAT)/db/MessageHandler.java \
	$(JAVA_SLEEPYCAT)/db/MetaCheckSumFailException.java \
	$(JAVA_SLEEPYCAT)/db/MultipleNIODataEntry.java \
	$(JAVA_SLEEPYCAT)/db/MultipleKeyNIODataEntry.java \
	$(JAVA_SLEEPYCAT)/db/MultipleRecnoNIODataEntry.java \
	$(JAVA_SLEEPYCAT)/db/MultipleDataEntry.java \
	$(JAVA_SLEEPYCAT)/db/MultipleEntry.java \
	$(JAVA_SLEEPYCAT)/db/MultipleKeyDataEntry.java \
	$(JAVA_SLEEPYCAT)/db/MultipleRecnoDataEntry.java \
	$(JAVA_SLEEPYCAT)/db/MutexStats.java \
	$(JAVA_SLEEPYCAT)/db/OperationStatus.java \
	$(JAVA_SLEEPYCAT)/db/PanicHandler.java \
	$(JAVA_SLEEPYCAT)/db/PartitionHandler.java \
	$(JAVA_SLEEPYCAT)/db/PreparedTransaction.java \
	$(JAVA_SLEEPYCAT)/db/QueueStats.java \
	$(JAVA_SLEEPYCAT)/db/RecordNumberAppender.java \
	$(JAVA_SLEEPYCAT)/db/RecoveryOperation.java \
	$(JAVA_SLEEPYCAT)/db/RegionResourceType.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationConfig.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationDuplicateMasterException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationHandleDeadException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationHoldElectionException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationHostAddress.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationJoinFailureException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationLeaseExpiredException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationLockoutException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationManagerAckPolicy.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationManagerConnectionStatus.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationManagerMessageDispatch.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationManagerSite.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationManagerSiteConfig.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationManagerSiteInfo.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationManagerStats.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationManagerStartPolicy.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationSiteUnavailableException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationStats.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationStatus.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationTimeoutType.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationTransport.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationViewHandler.java \
	$(JAVA_SLEEPYCAT)/db/RunRecoveryException.java \
	$(JAVA_SLEEPYCAT)/db/SecondaryConfig.java \
	$(JAVA_SLEEPYCAT)/db/SecondaryCursor.java \
	$(JAVA_SLEEPYCAT)/db/SecondaryDatabase.java \
	$(JAVA_SLEEPYCAT)/db/SecondaryKeyCreator.java \
	$(JAVA_SLEEPYCAT)/db/SecondaryMultiKeyCreator.java \
	$(JAVA_SLEEPYCAT)/db/Sequence.java \
	$(JAVA_SLEEPYCAT)/db/SequenceConfig.java \
	$(JAVA_SLEEPYCAT)/db/SequenceStats.java \
	$(JAVA_SLEEPYCAT)/db/Slice.java \
	$(JAVA_SLEEPYCAT)/db/SliceCorruptException.java \
	$(JAVA_SLEEPYCAT)/db/StatsConfig.java \
	$(JAVA_SLEEPYCAT)/db/Transaction.java \
	$(JAVA_SLEEPYCAT)/db/TransactionConfig.java \
	$(JAVA_SLEEPYCAT)/db/TransactionStats.java \
	$(JAVA_SLEEPYCAT)/db/TransactionStatus.java \
	$(JAVA_SLEEPYCAT)/db/VerboseConfig.java \
	$(JAVA_SLEEPYCAT)/db/VerifyConfig.java \
	$(JAVA_SLEEPYCAT)/db/VersionMismatchException.java \
	$(JAVA_SLEEPYCAT)/db/internal/Db.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbConstants.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbChannel.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbEnv.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbLock.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbLogc.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbMpoolFile.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbSequence.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbSite.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbStream.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbTxn.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbUtil.java \
	$(JAVA_SLEEPYCAT)/db/internal/Dbc.java \
	$(JAVA_SLEEPYCAT)/db/internal/db_java.java \
	$(JAVA_SLEEPYCAT)/db/internal/db_javaJNI.java \
	$(JAVA_SLEEPYCAT)/persist/BasicCursor.java \
	$(JAVA_SLEEPYCAT)/persist/BasicIndex.java \
	$(JAVA_SLEEPYCAT)/persist/BasicIterator.java \
	$(JAVA_SLEEPYCAT)/persist/DataValueAdapter.java \
	$(JAVA_SLEEPYCAT)/persist/DatabaseNamer.java \
	$(JAVA_SLEEPYCAT)/persist/EntityCursor.java \
	$(JAVA_SLEEPYCAT)/persist/EntityIndex.java \
	$(JAVA_SLEEPYCAT)/persist/EntityJoin.java \
	$(JAVA_SLEEPYCAT)/persist/EntityStore.java \
	$(JAVA_SLEEPYCAT)/persist/EntityValueAdapter.java \
	$(JAVA_SLEEPYCAT)/persist/ForwardCursor.java \
	$(JAVA_SLEEPYCAT)/persist/IndexNotAvailableException.java \
	$(JAVA_SLEEPYCAT)/persist/KeySelector.java \
	$(JAVA_SLEEPYCAT)/persist/KeyValueAdapter.java \
	$(JAVA_SLEEPYCAT)/persist/KeysIndex.java \
	$(JAVA_SLEEPYCAT)/persist/PrimaryIndex.java \
	$(JAVA_SLEEPYCAT)/persist/PrimaryKeyValueAdapter.java \
	$(JAVA_SLEEPYCAT)/persist/SecondaryIndex.java \
	$(JAVA_SLEEPYCAT)/persist/StoreConfig.java \
	$(JAVA_SLEEPYCAT)/persist/StoreConfigBeanInfo.java \
	$(JAVA_SLEEPYCAT)/persist/StoreExistsException.java \
	$(JAVA_SLEEPYCAT)/persist/StoreNotFoundException.java \
	$(JAVA_SLEEPYCAT)/persist/SubIndex.java \
	$(JAVA_SLEEPYCAT)/persist/SubIndexCursor.java \
	$(JAVA_SLEEPYCAT)/persist/ValueAdapter.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/Conversion.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/Converter.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/DeletedClassException.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/Deleter.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/EntityConverter.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/EvolveConfig.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/EvolveConfigBeanInfo.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/EvolveEvent.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/EvolveInternal.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/EvolveListener.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/EvolveStats.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/IncompatibleClassException.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/Mutation.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/Mutations.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/Renamer.java \
	$(JAVA_SLEEPYCAT)/persist/impl/AbstractInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Accessor.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Catalog.java \
	$(JAVA_SLEEPYCAT)/persist/impl/CollectionProxy.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ComplexFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ComparatorCatalog.java \
	$(JAVA_SLEEPYCAT)/persist/impl/CompositeKeyFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ConverterReader.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Enhanced.java \
	$(JAVA_SLEEPYCAT)/persist/impl/EnhancedAccessor.java \
	$(JAVA_SLEEPYCAT)/persist/impl/EntityInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/EntityOutput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/EnumFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Evolver.java \
	$(JAVA_SLEEPYCAT)/persist/impl/FieldInfo.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Format.java \
	$(JAVA_SLEEPYCAT)/persist/impl/KeyLocation.java \
	$(JAVA_SLEEPYCAT)/persist/impl/MapProxy.java \
	$(JAVA_SLEEPYCAT)/persist/impl/NonPersistentFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ObjectArrayFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PersistCatalog.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PersistComparator.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PersistEntityBinding.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PersistKeyAssigner.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PersistKeyBinding.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PersistKeyCreator.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PrimitiveArrayFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ProxiedFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RawAbstractInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RawAccessor.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RawArrayInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RawComplexInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RawSingleInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ReadOnlyCatalog.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Reader.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RecordInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RecordOutput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ReflectionAccessor.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RefreshException.java \
	$(JAVA_SLEEPYCAT)/persist/impl/SimpleCatalog.java \
	$(JAVA_SLEEPYCAT)/persist/impl/SimpleFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Store.java \
	$(JAVA_SLEEPYCAT)/persist/impl/StoredModel.java \
	$(JAVA_SLEEPYCAT)/persist/impl/VisitedObjects.java \
	$(JAVA_SLEEPYCAT)/persist/impl/WidenerInput.java \
	$(JAVA_SLEEPYCAT)/persist/model/AnnotationModel.java \
	$(JAVA_SLEEPYCAT)/persist/model/ClassMetadata.java \
	$(JAVA_SLEEPYCAT)/persist/model/DeleteAction.java \
	$(JAVA_SLEEPYCAT)/persist/model/Entity.java \
	$(JAVA_SLEEPYCAT)/persist/model/EntityMetadata.java \
	$(JAVA_SLEEPYCAT)/persist/model/EntityModel.java \
	$(JAVA_SLEEPYCAT)/persist/model/FieldMetadata.java \
	$(JAVA_SLEEPYCAT)/persist/model/KeyField.java \
	$(JAVA_SLEEPYCAT)/persist/model/ModelInternal.java \
	$(JAVA_SLEEPYCAT)/persist/model/NotPersistent.java \
	$(JAVA_SLEEPYCAT)/persist/model/NotTransient.java \
	$(JAVA_SLEEPYCAT)/persist/model/Persistent.java \
	$(JAVA_SLEEPYCAT)/persist/model/PersistentProxy.java \
	$(JAVA_SLEEPYCAT)/persist/model/PrimaryKey.java \
	$(JAVA_SLEEPYCAT)/persist/model/PrimaryKeyMetadata.java \
	$(JAVA_SLEEPYCAT)/persist/model/Relationship.java \
	$(JAVA_SLEEPYCAT)/persist/model/SecondaryKey.java \
	$(JAVA_SLEEPYCAT)/persist/model/SecondaryKeyMetadata.java \
	$(JAVA_SLEEPYCAT)/persist/raw/RawField.java \
	$(JAVA_SLEEPYCAT)/persist/raw/RawObject.java \
	$(JAVA_SLEEPYCAT)/persist/raw/RawStore.java \
	$(JAVA_SLEEPYCAT)/persist/raw/RawType.java \
	$(JAVA_SLEEPYCAT)/util/ClassResolver.java \
	$(JAVA_SLEEPYCAT)/util/ConfigBeanInfoBase.java \
	$(JAVA_SLEEPYCAT)/util/ErrorBuffer.java \
	$(JAVA_SLEEPYCAT)/util/ExceptionUnwrapper.java \
	$(JAVA_SLEEPYCAT)/util/ExceptionWrapper.java \
	$(JAVA_SLEEPYCAT)/util/FastInputStream.java \
	$(JAVA_SLEEPYCAT)/util/FastOutputStream.java \
	$(JAVA_SLEEPYCAT)/util/IOExceptionWrapper.java \
	$(JAVA_SLEEPYCAT)/util/PackedInteger.java \
	$(JAVA_SLEEPYCAT)/util/RuntimeExceptionWrapper.java \
	$(JAVA_SLEEPYCAT)/util/UtfOps.java \
	$(JAVA_SLEEPYCAT)/util/keyrange/KeyRange.java \
	$(JAVA_SLEEPYCAT)/util/keyrange/KeyRangeException.java \
	$(JAVA_SLEEPYCAT)/util/keyrange/RangeCursor.java

JAVA_EXSRCS=\
	$(JAVA_EXDIR)/collections/access/AccessExample.java \
	$(JAVA_EXDIR)/collections/hello/HelloDatabaseWorld.java \
	$(JAVA_EXDIR)/collections/ship/basic/PartData.java \
	$(JAVA_EXDIR)/collections/ship/basic/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/basic/Sample.java \
	$(JAVA_EXDIR)/collections/ship/basic/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/basic/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/basic/ShipmentData.java \
	$(JAVA_EXDIR)/collections/ship/basic/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/basic/SupplierData.java \
	$(JAVA_EXDIR)/collections/ship/basic/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/basic/Weight.java \
	$(JAVA_EXDIR)/collections/ship/entity/Part.java \
	$(JAVA_EXDIR)/collections/ship/entity/PartData.java \
	$(JAVA_EXDIR)/collections/ship/entity/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/entity/Sample.java \
	$(JAVA_EXDIR)/collections/ship/entity/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/entity/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/entity/Shipment.java \
	$(JAVA_EXDIR)/collections/ship/entity/ShipmentData.java \
	$(JAVA_EXDIR)/collections/ship/entity/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/entity/Supplier.java \
	$(JAVA_EXDIR)/collections/ship/entity/SupplierData.java \
	$(JAVA_EXDIR)/collections/ship/entity/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/entity/Weight.java \
	$(JAVA_EXDIR)/collections/ship/factory/Part.java \
	$(JAVA_EXDIR)/collections/ship/factory/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/factory/Sample.java \
	$(JAVA_EXDIR)/collections/ship/factory/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/factory/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/factory/Shipment.java \
	$(JAVA_EXDIR)/collections/ship/factory/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/factory/Supplier.java \
	$(JAVA_EXDIR)/collections/ship/factory/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/factory/Weight.java \
	$(JAVA_EXDIR)/collections/ship/index/PartData.java \
	$(JAVA_EXDIR)/collections/ship/index/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/index/Sample.java \
	$(JAVA_EXDIR)/collections/ship/index/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/index/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/index/ShipmentData.java \
	$(JAVA_EXDIR)/collections/ship/index/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/index/SupplierData.java \
	$(JAVA_EXDIR)/collections/ship/index/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/index/Weight.java \
	$(JAVA_EXDIR)/collections/ship/marshal/MarshalledEnt.java \
	$(JAVA_EXDIR)/collections/ship/marshal/MarshalledKey.java \
	$(JAVA_EXDIR)/collections/ship/marshal/Part.java \
	$(JAVA_EXDIR)/collections/ship/marshal/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/marshal/Sample.java \
	$(JAVA_EXDIR)/collections/ship/marshal/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/marshal/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/marshal/Shipment.java \
	$(JAVA_EXDIR)/collections/ship/marshal/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/marshal/Supplier.java \
	$(JAVA_EXDIR)/collections/ship/marshal/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/marshal/Weight.java \
	$(JAVA_EXDIR)/collections/ship/sentity/Part.java \
	$(JAVA_EXDIR)/collections/ship/sentity/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/sentity/Sample.java \
	$(JAVA_EXDIR)/collections/ship/sentity/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/sentity/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/sentity/Shipment.java \
	$(JAVA_EXDIR)/collections/ship/sentity/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/sentity/Supplier.java \
	$(JAVA_EXDIR)/collections/ship/sentity/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/sentity/Weight.java \
	$(JAVA_EXDIR)/collections/ship/tuple/Part.java \
	$(JAVA_EXDIR)/collections/ship/tuple/PartData.java \
	$(JAVA_EXDIR)/collections/ship/tuple/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/tuple/Sample.java \
	$(JAVA_EXDIR)/collections/ship/tuple/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/tuple/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/tuple/Shipment.java \
	$(JAVA_EXDIR)/collections/ship/tuple/ShipmentData.java \
	$(JAVA_EXDIR)/collections/ship/tuple/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/tuple/Supplier.java \
	$(JAVA_EXDIR)/collections/ship/tuple/SupplierData.java \
	$(JAVA_EXDIR)/collections/ship/tuple/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/tuple/Weight.java \
	$(JAVA_EXDIR)/db/AccessExample.java \
	$(JAVA_EXDIR)/db/BtRecExample.java \
	$(JAVA_EXDIR)/db/BulkAccessExample.java \
	$(JAVA_EXDIR)/db/BulkAccessNIOExample.java \
	$(JAVA_EXDIR)/db/BulkExample.java \
	$(JAVA_EXDIR)/db/EnvExample.java \
	$(JAVA_EXDIR)/db/ExternalFileExample.java \
	$(JAVA_EXDIR)/db/GettingStarted/ExampleDatabaseLoad.java \
	$(JAVA_EXDIR)/db/GettingStarted/ExampleDatabaseRead.java \
	$(JAVA_EXDIR)/db/GettingStarted/Inventory.java \
	$(JAVA_EXDIR)/db/GettingStarted/InventoryBinding.java \
	$(JAVA_EXDIR)/db/GettingStarted/ItemNameKeyCreator.java \
	$(JAVA_EXDIR)/db/GettingStarted/MyDbs.java \
	$(JAVA_EXDIR)/db/GettingStarted/Vendor.java \
	$(JAVA_EXDIR)/db/LockExample.java \
	$(JAVA_EXDIR)/db/SequenceExample.java \
	$(JAVA_EXDIR)/db/TpcbExample.java \
	$(JAVA_EXDIR)/db/repquote/RepConfig.java \
	$(JAVA_EXDIR)/db/repquote/RepQuoteEnvironment.java \
	$(JAVA_EXDIR)/db/repquote/RepQuoteExample.java \
	$(JAVA_EXDIR)/db/repquote/RepRemoteHost.java \
	$(JAVA_EXDIR)/db/repquote_gsg/RepConfig.java \
	$(JAVA_EXDIR)/db/repquote_gsg/RepQuoteEnvironment.java \
	$(JAVA_EXDIR)/db/repquote_gsg/RepQuoteExampleGSG.java \
	$(JAVA_EXDIR)/db/repquote_gsg/RepWrforConfig.java \
	$(JAVA_EXDIR)/db/repquote_gsg/RepQuoteWrforExampleGSG.java \
	$(JAVA_EXDIR)/db/repquote_gsg/SimpleConfig.java \
	$(JAVA_EXDIR)/db/repquote_gsg/SimpleTxn.java \
	$(JAVA_EXDIR)/db/txn/DBWriter.java \
	$(JAVA_EXDIR)/db/txn/PayloadData.java \
	$(JAVA_EXDIR)/db/txn/TxnGuide.java \
	$(JAVA_EXDIR)/db/txn/TxnGuideInMemory.java \
	$(JAVA_EXDIR)/persist/CustomKeyOrderExample.java \
	$(JAVA_EXDIR)/persist/DplDump.java \
	$(JAVA_EXDIR)/persist/EventExample.java \
	$(JAVA_EXDIR)/persist/EventExampleDPL.java \
	$(JAVA_EXDIR)/persist/PersonExample.java \
	$(JAVA_EXDIR)/persist/gettingStarted/SimpleDA.java \
	$(JAVA_EXDIR)/persist/gettingStarted/SimpleEntityClass.java \
	$(JAVA_EXDIR)/persist/gettingStarted/SimpleStoreGet.java \
	$(JAVA_EXDIR)/persist/gettingStarted/SimpleStorePut.java \
	$(JAVA_EXDIR)/persist/txn/PayloadDataEntity.java \
	$(JAVA_EXDIR)/persist/txn/StoreWriter.java \
	$(JAVA_EXDIR)/persist/txn/TxnGuideDPL.java

JAVA_DEMOSRCS=\
	$(JAVA_DEMODIR)/DbManagerFactory.java \
	$(JAVA_DEMODIR)/ParkingDemo.java \
	$(JAVA_DEMODIR)/data/Ticket.java \
	$(JAVA_DEMODIR)/data/TicketLog.java \
	$(JAVA_DEMODIR)/dbinterface/BasicDbManager.java \
	$(JAVA_DEMODIR)/dbinterface/BasicTicketDAO.java \
	$(JAVA_DEMODIR)/dbinterface/BasicTicketLogDAO.java \
	$(JAVA_DEMODIR)/dbinterface/CloseableIterable.java \
	$(JAVA_DEMODIR)/dbinterface/DbManager.java \
	$(JAVA_DEMODIR)/dbinterface/DPLDbManager.java \
	$(JAVA_DEMODIR)/dbinterface/DPLTicketDAO.java \
	$(JAVA_DEMODIR)/dbinterface/DPLTicketLogDAO.java \
	$(JAVA_DEMODIR)/dbinterface/EnvDbManager.java \
	$(JAVA_DEMODIR)/dbinterface/SQLDbManager.java \
	$(JAVA_DEMODIR)/dbinterface/SQLTicketDAO.java \
	$(JAVA_DEMODIR)/dbinterface/SQLTicketLogDAO.java \
	$(JAVA_DEMODIR)/dbinterface/TicketDAO.java \
	$(JAVA_DEMODIR)/dbinterface/TicketLogDAO.java \
	$(JAVA_DEMODIR)/park/Meter.java \
	$(JAVA_DEMODIR)/park/Reporting.java

JAVA_GUISRCS=\
	$(JAVA_GUIDIR)/BDBInitializable.java \
	$(JAVA_GUIDIR)/BDBState.java \
	$(JAVA_GUIDIR)/db_gui.java \
	$(JAVA_GUIDIR)/EnableRunnable.java \
	$(JAVA_GUIDIR)/MessageBuffer.java \
	$(JAVA_GUIDIR)/datapage/BDBDataInitializable.java \
	$(JAVA_GUIDIR)/datapage/BtreeHashController.java \
	$(JAVA_GUIDIR)/datapage/DataAccessController.java \
	$(JAVA_GUIDIR)/datapage/HeapController.java \
	$(JAVA_GUIDIR)/datapage/QueueController.java \
	$(JAVA_GUIDIR)/datapage/RecnoController.java \
	$(JAVA_GUIDIR)/datapage/TransactionController.java \
	$(JAVA_GUIDIR)/dbpage/BDBDbInitializable.java \
	$(JAVA_GUIDIR)/dbpage/CompactTask.java \
	$(JAVA_GUIDIR)/dbpage/DatabaseCompactController.java \
	$(JAVA_GUIDIR)/dbpage/DatabaseCreateController.java \
	$(JAVA_GUIDIR)/dbpage/DatabaseOpenController.java \
	$(JAVA_GUIDIR)/dbpage/DatabasePageController.java \
	$(JAVA_GUIDIR)/dbpage/DatabaseRemoveController.java \
	$(JAVA_GUIDIR)/dbpage/DatabaseRenameController.java \
	$(JAVA_GUIDIR)/envpage/BDBEnvInitializable.java \
	$(JAVA_GUIDIR)/envpage/BackupTask.java \
	$(JAVA_GUIDIR)/envpage/EnvConfig.java \
	$(JAVA_GUIDIR)/envpage/EnvironmentBackupController.java \
	$(JAVA_GUIDIR)/envpage/EnvironmentCreateController.java \
	$(JAVA_GUIDIR)/envpage/EnvironmentOpenController.java \
	$(JAVA_GUIDIR)/envpage/EnvironmentPageController.java \
	$(JAVA_GUIDIR)/envpage/EnvironmentRecoveryController.java \
	$(JAVA_GUIDIR)/envpage/EnvironmentRemoveLogController.java \
	$(JAVA_GUIDIR)/envpage/EnvironmentUpgradeController.java \
	$(JAVA_GUIDIR)/envpage/EnvironmentVerifyDataController.java \
	$(JAVA_GUIDIR)/envpage/EnvironmentVerifyLogController.java \
	$(JAVA_GUIDIR)/envpage/RecoveryTask.java \
	$(JAVA_GUIDIR)/envpage/RemoveLogTask.java \
	$(JAVA_GUIDIR)/envpage/UpgradeTask.java \
	$(JAVA_GUIDIR)/envpage/VerifyDataTask.java \
	$(JAVA_GUIDIR)/envpage/VerifyLogTask.java

JAVA_GUIDATAFXML=\
	$(JAVA_GUIDIR)/datapage/FXMLBtreeHash.fxml \
	$(JAVA_GUIDIR)/datapage/FXMLDataAccess.fxml \
	$(JAVA_GUIDIR)/datapage/FXMLHeap.fxml \
	$(JAVA_GUIDIR)/datapage/FXMLQueue.fxml \
	$(JAVA_GUIDIR)/datapage/FXMLRecno.fxml \
	$(JAVA_GUIDIR)/datapage/FXMLTransaction.fxml 

JAVA_GUIDBFXML=\
	$(JAVA_GUIDIR)/dbpage/FXMLDatabaseCompactTab.fxml \
	$(JAVA_GUIDIR)/dbpage/FXMLDatabaseCreateTab.fxml \
	$(JAVA_GUIDIR)/dbpage/FXMLDatabaseOpenTab.fxml \
	$(JAVA_GUIDIR)/dbpage/FXMLDatabasePage.fxml \
	$(JAVA_GUIDIR)/dbpage/FXMLDatabaseRemoveTab.fxml \
	$(JAVA_GUIDIR)/dbpage/FXMLDatabaseRenameTab.fxml 

JAVA_GUIENVFXML=\
	$(JAVA_GUIDIR)/envpage/FXMLEnvironmentBackupTab.fxml \
	$(JAVA_GUIDIR)/envpage/FXMLEnvironmentCreateTab.fxml \
	$(JAVA_GUIDIR)/envpage/FXMLEnvironmentOpenTab.fxml \
	$(JAVA_GUIDIR)/envpage/FXMLEnvironmentPage.fxml \
	$(JAVA_GUIDIR)/envpage/FXMLEnvironmentRecoveryTab.fxml \
	$(JAVA_GUIDIR)/envpage/FXMLEnvironmentRemoveLogTab.fxml \
	$(JAVA_GUIDIR)/envpage/FXMLEnvironmentUpgradeTab.fxml \
	$(JAVA_GUIDIR)/envpage/FXMLEnvironmentVerifyDataTab.fxml \
	$(JAVA_GUIDIR)/envpage/FXMLEnvironmentVerifyLogTab.fxml

JAVA_GUITESTSRCS=\
	$(JAVA_GUITESTDIR)/BDBGUITestConfig.java \
	$(JAVA_GUITESTDIR)/BDBGUITestRunner.java \
	$(JAVA_GUITESTDIR)/ComboBoxTask.java \
	$(JAVA_GUITESTDIR)/DataAccessPageTests.java \
	$(JAVA_GUITESTDIR)/DatabasePageTests.java \
	$(JAVA_GUITESTDIR)/DisplayAccessPageTask.java \
	$(JAVA_GUITESTDIR)/EnvironmentPageTests.java

SQL_OBJS=\
	 sqlite3.lo $(C_OBJS)

STL_OBJS=\
	dbstl_container.lo dbstl_resource_manager.lo

TCL_OBJS=\
	tcl_compat.lo tcl_db.lo tcl_db_pkg.lo tcl_dbcursor.lo tcl_dbstream.lo \
	tcl_env.lo tcl_internal.lo tcl_lock.lo tcl_log.lo tcl_mp.lo \
	tcl_mutex.lo tcl_rep.lo tcl_seq.lo tcl_txn.lo tcl_util.lo

TEST_MICRO_OBJS=\
	b_curalloc.lo b_curwalk.lo b_del.lo b_get.lo b_inmem.lo b_latch.lo \
	b_load.lo b_open.lo b_put.lo b_recover.lo b_txn.lo b_txn_write.lo \
	b_uname.lo b_util.lo b_workload.lo test_micro.lo util_arg.lo

UTIL_PROGS=\
	 \
	db_archive db_checkpoint db_convert db_deadlock db_dump \
	db_hotbackup db_load db_log_verify db_printlog db_recover \
	db_replicate db_stat db_tuner db_upgrade db_verify

##################################################
# List of files installed into the library directory.
##################################################
LIB_INSTALL_FILE_LIST=\
	$(libdb) \
	$(libso) \
	$(libso_default) \
	$(libso_major) \
	$(libdb_version) \
	$(libcxx) \
	$(libxso) \
	$(libxso_default) \
	$(libxso_major) \
	$(libcxx_version) \
	$(libsql) \
	$(libsqlso) \
	$(libsqlso_default) \
	$(libsqlso_major) \
	$(libsql_version) \
	$(libstl) \
	$(libstlso) \
	$(libstlso_default) \
	$(libstlso_major) \
	$(libstl_version) \
	$(libtcl) \
	$(libtso) \
	$(libtso_default) \
	$(libtso_major) \
	$(libtcl_version) \
	$(libjso) \
	$(libjso_default) \
	$(libjso_g) \
	$(libjso_major) \
	$(libjso_static) \
	$(libj_exjarfile) \
	$(libj_demojarfile) \
	$(libj_guijarfile) \
	$(libj_jarfile) \
	$(libso_target) $(libdb) 

##################################################
# Note: "all" must be the first target in the Makefile.
##################################################
all: library_build

install: all library_install

##################################################
# Library and standard utilities build.
##################################################
library_build: $(libso_target) $(libdb)  $(UTIL_PROGS)

# Static C library named libdb.a.
$(libdb): $(DEF_LIB)

# Real static C library.
$(libdb_version): $(C_OBJS)
	$(AR) cr $@ $(C_OBJS)
	$(RANLIB) $@
	$(RM) $(libdb)
	$(LN) -s $(libdb_version) $(libdb)

# Shared C library.
$(libso_target): $(C_OBJS)
	$(SOLINK) $(SOFLAGS) $(LDFLAGS) -o $@ $(C_OBJS) \
		$(LIBCSO_LIBS)
	$(RM) $(libdb)
	$(LN) -s .libs/$(libdb_version) $(libdb)

# Static C++ library named libdb_cxx.a.
$(libcxx): $(DEF_LIB_CXX)

# Real static C++ library.
$(libcxx_version): $(CXX_OBJS) $(C_OBJS)
	$(AR) cr $@ $(CXX_OBJS) $(C_OBJS)
	$(RANLIB) $@
	$(RM) $(libcxx)
	$(LN) -s $(libcxx_version) $(libcxx)

# Shared C++ library.
$(libxso_target): $(CXX_OBJS) $(C_OBJS)
	$(XSOLINK) $(SOFLAGS) $(LDFLAGS) \
	    -o $@ $(CXX_OBJS) $(C_OBJS) $(LIBXSO_LIBS)
	$(RM) $(libcxx)
	$(LN) -s .libs/$(libcxx_version) $(libcxx)

# Static SQL library named libdb_sql.a.
$(libsql): $(DEF_LIB_SQL)

# Static SQL library.
$(libsql_version): $(SQL_OBJS)
	$(AR) cr $@ $(SQL_OBJS)
	$(RANLIB) $@
	$(RM) $(libsql)
	$(LN) -s $(libsql_version) $(libsql)

# Shared SQL library.
$(libsqlso_target): $(SQL_OBJS)
	$(SOLINK) $(SOFLAGS) $(LDFLAGS) -o $@ $(SQL_OBJS) $(LIBSQLSO_LIBS)
	$(RM) $(libsql)
	$(LN) -s .libs/$(libsql_version) $(libsql)

dbsql: shell.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) shell.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

jdbc: $(DEF_LIB)
	@(test -d jdbc && cd jdbc && PWD='.' env CPPFLAGS="" JMODSUFFIX=.so $(MAKE))

sql-test: $(DEF_LIB)
	@(cd sql && $(MAKE) LTLINK_EXTRAS="../$(DEF_LIB) $(LIBS)" testfixture)

# SQL API header file to be installed
# Some configurations of Solaris make don't handle $< as an explicit dependency
# so duplicate the name in the rule.
dbsql.h: $(langdir)/sql/generated/sqlite3.h
	$(CP) $(langdir)/sql/generated/sqlite3.h $@

# SQLite drop-in replacements
$(libsqlite): $(SQL_OBJS)
	$(AR) cr $@ $(SQL_OBJS)
	$(RANLIB) $@

$(libsqliteso_target): $(SQL_OBJS)
	$(SOLINK) $(SOFLAGS) $(LDFLAGS) -version-info "8:6:8" -o $@ \
		$(SQL_OBJS) $(LIBCSO_LIBS)

sqlite3: shell.lo $(DEF_LIB_SQLITE)
	$(CCLINK) -o $@ $(LDFLAGS) shell.lo $(DEF_LIB_SQLITE) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

# Static STL library named libdb_stl.a.
$(libstl): $(DEF_LIB_STL)

# Real static STL library.
$(libstl_version): $(STL_OBJS) $(CXX_OBJS) $(C_OBJS)
	$(AR) cr $@ $(STL_OBJS) $(CXX_OBJS) $(C_OBJS)
	$(RANLIB) $@
	$(RM) $(libstl)
	$(LN) -s $(libstl_version) $(libstl)

# Shared STL library.
$(libstlso_target): $(STL_OBJS) $(CXX_OBJS) $(C_OBJS)
	$(XSOLINK) $(SOFLAGS) $(LDFLAGS) -o $@ $(STL_OBJS) \
	    $(LIBSTLSO_LIBS) $(CXX_OBJS) $(C_OBJS) $(LIBXSO_LIBS)
	$(RM) $(libstl)
	$(LN) -s .libs/$(libstl_version) $(libstl)

# Shared Java library.
$(libjso_target): $(JAVA_OBJS) $(C_OBJS)
	$(SOLINK) -shrext .so $(SOFLAGS) $(LDFLAGS) \
	    -o $@ $(JAVA_OBJS) $(C_OBJS) $(LIBJSO_LIBS)

# Static Tcl library
$(libtcl): $(DEF_LIB_TCL)

# Real static Tcl library.
$(libtcl_version): $(TCL_OBJS) $(C_OBJS)
	$(AR) cr $@ $(TCL_OBJS) $(C_OBJS)
	$(RANLIB) $@
	$(RM) $(libtcl)
	$(LN) -s $(libtcl_version) $(libtcl)

# Shared Tcl library.
$(libtso_target): $(TCL_OBJS) $(C_OBJS)
	$(SOLINK) -module $(SOFLAGS) $(LDFLAGS) \
	    -o $@ $(TCL_OBJS) $(C_OBJS) $(LIBTSO_LIBS)
	$(RM) $(libtcl)
	$(LN) -s .libs/$(libtcl_version) $(libtcl)

##################################################
# Creating individual dependencies and actions for building class
# files is possible, but it is very messy and error prone.
##################################################
java:	$(libj_jarfile) $(libj_exjarfile)

$(libj_jarfile): $(JAVA_DBSRCS)
	@test -d $(JAVA_CLASSTOP) || \
	   ($(MKDIR) -p $(JAVA_CLASSTOP) && \
	   $(CHMOD) $(dmode) $(JAVA_CLASSTOP))
	$(JAVAC) -d $(JAVA_CLASSTOP) $(JAVACFLAGS) $(JAVA_DBSRCS)
	cd $(JAVA_CLASSTOP) && \
	     $(JAR) cfm ../$(libj_jarfile) ../$(JAVA_MANIFEST) ./com/sleepycat

$(libj_exjarfile): $(libj_jarfile) $(JAVA_EXSRCS)
	@test -d $(JAVA_EXCLASSTOP) || \
	   ($(MKDIR) -p $(JAVA_EXCLASSTOP) && \
		$(CHMOD) $(dmode) $(JAVA_EXCLASSTOP))
	$(JAVAC) -classpath $(libj_jarfile) -d $(JAVA_EXCLASSTOP) \
		$(JAVACFLAGS) $(JAVA_EXSRCS)
	cd $(JAVA_EXCLASSTOP) && $(JAR) cf ../$(libj_exjarfile) .

$(libj_demojarfile): $(libj_jarfile) $(JAVA_DEMOSRCS)
	@if test $(JDK_VERSION) -gt 6 ; then \
		test -d $(JAVA_DEMOCLASSTOP) || \
		    ($(MKDIR) -p $(JAVA_DEMOCLASSTOP) && \
		        $(CHMOD) $(dmode) $(JAVA_DEMOCLASSTOP)) ; \
		$(JAVAC) -classpath $(libj_jarfile) -d $(JAVA_DEMOCLASSTOP) \
		    $(JAVACFLAGS) $(JAVA_DEMOSRCS) ; \
		cd $(JAVA_DEMOCLASSTOP) && $(JAR) cf ../$(libj_demojarfile) . ; \
	else \
		echo "$(libj_demojarfile) requires JDK 7 or above." ; \
	fi

db_gui:	$(libj_guijarfile)

$(libj_guijarfile): $(libj_jarfile) $(JAVA_GUISRCS)
		test -d $(JAVA_GUICLASSTOP) || \
	   	    ($(MKDIR) -p $(JAVA_GUICLASSTOP) && \
			$(CHMOD) $(dmode) $(JAVA_GUICLASSTOP)) ; \
		$(JAVAC) -classpath $(libj_jarfile) -d $(JAVA_GUICLASSTOP) \
			$(JAVACFLAGS) $(JAVA_GUISRCS) ; \
		$(CP) $(JAVA_GUIDATAFXML) $(JAVA_GUICLASSTOP)/db_gui/datapage ; \
		$(CP) $(JAVA_GUIDBFXML) $(JAVA_GUICLASSTOP)/db_gui/dbpage ; \
		$(CP) $(JAVA_GUIDATAFXML) $(JAVA_GUICLASSTOP)/db_gui/envpage ; \
		cd $(JAVA_GUICLASSTOP) && $(JAR) cf ../$(libj_guijarfile) . ; 

db_guitest: $(libj_guitestjarfile)

$(libj_guitestjarfile): $(libj_guijarfile) $(JAVA_GUITESTSRCS)
		test -d $(JAVA_GUITESTCLASSTOP) || \
	   	    ($(MKDIR) -p $(JAVA_GUITESTCLASSTOP) && \
			$(CHMOD) $(dmode) $(JAVA_GUICLASSTOP)) ; \
		nojavac -classpath "$(libj_jarfile):$(libj_guijarfile):"$(GUITESTCLASSPATH) -d $(JAVA_GUITESTCLASSTOP) \
			$(JAVACFLAGS) $(JAVA_GUITESTSRCS) ; \
		cd $(JAVA_GUITESTCLASSTOP) && $(JAR) cf ../$(libj_guitestjarfile) . ;


##################################################
# Utilities
##################################################
COMMON_UTIL_OBJS=\
	  util_arg.lo util_cache.lo util_env.lo util_log.lo util_sig.lo \
	  util_ver_check.lo

db_archive: db_archive.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_archive.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_checkpoint: db_checkpoint.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_checkpoint.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_convert: db_convert.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_convert.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_deadlock: db_deadlock.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_deadlock.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_dump: db_dump.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_dump.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_dump185: db_dump185.lo 
	$(CCLINK) -o $@ $(LDFLAGS) db_dump185.lo  $(DB185LIB)
	$(POSTLINK) $@

db_hotbackup: db_hotbackup.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_hotbackup.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_load: db_load.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_load.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_printlog: db_printlog.lo $(PRINT_OBJS) $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_printlog.lo $(PRINT_OBJS) $(COMMON_UTIL_OBJS) $(DEF_LIB) \
	    $(LIBS)
	$(POSTLINK) $@

db_recover: db_recover.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_recover.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_replicate: db_replicate.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_replicate.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_stat: db_stat.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_stat.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_tuner: db_tuner.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_tuner.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_upgrade: db_upgrade.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_upgrade.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_verify: db_verify.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_verify.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_log_verify: db_log_verify.lo $(COMMON_UTIL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_log_verify.lo $(COMMON_UTIL_OBJS) $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@
##################################################
# Library and standard utilities install.
##################################################
library_install: install_setup
library_install: install_include install_lib install_utilities  install_docs

uninstall: uninstall_include uninstall_lib uninstall_utilities  uninstall_docs

install_setup:
	@test -d $(DESTDIR)$(prefix) || \
	    ($(MKDIR) -p $(DESTDIR)$(prefix) && \
	    $(CHMOD) $(dmode) $(DESTDIR)$(prefix))

INCDOT=	db.h db_cxx.h 
install_include: $(INCDOT)
	@echo "Installing DB include files: $(DESTDIR)$(includedir) ..."
	@test -d $(DESTDIR)$(includedir) || \
	    ($(MKDIR) -p $(DESTDIR)$(includedir) && \
	    $(CHMOD) $(dmode) $(DESTDIR)$(includedir))
	@for f in $(INCDOT); do \
	    basef=`echo $$f | $(SED) 's,.*/,,'` ; \
	    $(RM) $(DESTDIR)$(includedir)/$$basef ; \
	    $(CP) -p $$f $(DESTDIR)$(includedir) ; \
	    $(CHMOD) $(fmode) $(DESTDIR)$(includedir)/$$basef ; \
	done

uninstall_include:
	@for f in $(INCDOT); do \
	    basef=`echo $$f | $(SED) 's,.*/,,'` ; \
	    $(RM) $(DESTDIR)$(includedir)/$$basef ; \
	done

install_lib:
	@echo "Installing DB library: $(DESTDIR)$(libdir) ..."
	@test -d $(DESTDIR)$(libdir) || \
	    ($(MKDIR) -p $(DESTDIR)$(libdir) && \
	    $(CHMOD) $(dmode) $(DESTDIR)$(libdir))
	@cd $(DESTDIR)$(libdir) && $(RM) $(LIB_INSTALL_FILE_LIST)
	@$(INSTALLER) $(libso_target) $(libdb) $(DESTDIR)$(libdir)
	@(cd $(DESTDIR)$(libdir) && \
	    test -f $(libso) && $(LN) -s $(libso) $(libso_default); \
	    test -f $(libso) && $(LN) -s $(libso) $(libso_major); \
	    test -f $(libxso) && $(LN) -s $(libxso) $(libxso_default); \
	    test -f $(libxso) && $(LN) -s $(libxso) $(libxso_major); \
	    test -f $(libsqlso) && $(LN) -s $(libsqlso) $(libsqlso_default); \
	    test -f $(libsqlso) && $(LN) -s $(libsqlso) $(libsqlso_major); \
	    test -f $(libstlso) && $(LN) -s $(libstlso) $(libstlso_default); \
	    test -f $(libstlso) && $(LN) -s $(libstlso) $(libstlso_major); \
	    test -f $(libtso) && $(LN) -s $(libtso) $(libtso_default); \
	    test -f $(libtso) && $(LN) -s $(libtso) $(libtso_major); \
	    test -f $(libjso) && $(LN) -s $(libjso) $(libjso_default); \
	    test -f $(libjso) && $(LN) -s $(libjso) $(libjso_major); \
	    test -f $(libjso) && $(LN) -s $(libjso) $(libjso_g)) || exit 0
	@(test -f $(libj_jarfile) && \
	    $(CP) $(libj_jarfile) $(DESTDIR)$(libdir) && \
	    $(CHMOD) $(fmode) $(DESTDIR)$(libdir)/$(libj_jarfile)) || exit 0
	@(test -f $(libj_guijarfile) && \
	    $(CP) $(libj_guijarfile) $(DESTDIR)$(libdir) && \
	    $(CHMOD) $(fmode) $(DESTDIR)$(libdir)/$(libj_guijarfile)) || exit 0
	@(test -d jdbc && cd jdbc && make install && cd -) || exit 0

uninstall_lib:
	@cd $(DESTDIR)$(libdir) && $(RM) $(LIB_INSTALL_FILE_LIST)
	@(test -d jdbc && cd jdbc && make uninstall && cd -) || exit 0

install_utilities:
	@echo "Installing DB utilities: $(DESTDIR)$(bindir) ..."
	@test -d $(DESTDIR)$(bindir) || \
	    ($(MKDIR) -p $(DESTDIR)$(bindir) && \
	    $(CHMOD) $(dmode) $(DESTDIR)$(bindir))
	@for i in $(UTIL_PROGS); do \
		test -f $$i.exe && i=$$i.exe; \
		e=`echo $$i | $(SED) '$(transform)'`; \
		$(RM) $(DESTDIR)$(bindir)/$$e; \
		$(INSTALLER) $$i $(DESTDIR)$(bindir)/$$e; \
		$(STRIP) $(DESTDIR)$(bindir)/$$e; \
		$(CHMOD) $(emode) $(DESTDIR)$(bindir)/$$e; \
	done

uninstall_utilities:
	@(cd $(DESTDIR)$(bindir); for i in $(UTIL_PROGS); do \
		i=`echo $$i | $(SED) '$(transform)'`; \
		test -f $$i.exe && i=$$i.exe; \
		$(RM) $$i; \
	done)

# We install csharp docs even on UNIX so we don't have a 
# broken link on the landing page.
DOCLIST=api_reference articles bdb-sql collections csharp gsg \
	gsg_db_rep gsg_db_server gsg_txn index.html installation \
	java license porting programmer_reference upgrading

install_docs:
	@echo "Installing documentation: $(DESTDIR)$(docdir) ..."
	@test -d $(DESTDIR)$(docdir) || \
	    ($(MKDIR) -p $(DESTDIR)$(docdir) && \
	    $(CHMOD) $(dmode) $(DESTDIR)$(docdir))
	@cd $(DESTDIR)$(docdir) && $(RM) -r $(DOCLIST)
	@cd $(topdir)/docs && $(CP) -pr $(DOCLIST) $(DESTDIR)$(docdir)/

uninstall_docs:
	@cd $(DESTDIR)$(docdir) && $(RM) -r $(DOCLIST)

##################################################
# Remaining standard Makefile targets.
##################################################
CLEAN_LIST=\
	StlTxnGuide TxnGuide TxnGuideInMemory \
	berkeley_db_cxxsvc berkeley_db_svc cutest \
	db_dump185 db_perf db_repsite db_reptest bdb_ha dbs ex_access \
	ex_advertising ex_ad_data ex_ad_event ex_apprec \
	ex_btrec ex_bulk ex_dbclient ex_env ex_external_file ex_firewall \
	ex_firewall_dest ex_firewall_msg ex_heap ex_lock ex_mpool  \
	ex_priority ex_priority_dest ex_priority_msg \
	ex_rep_base ex_rep_chan ex_rep_gsg_repmgr ex_rep_gsg_wrfor \
	ex_rep_gsg_simple ex_rep_mgr ex_sequence ex_slice ex_stream ex_thread \
	ex_toll_booth ex_toll_data ex_toll_event ex_tpcb \
	example_database_load example_database_read excxx_access \
	excxx_btrec excxx_bulk excxx_env excxx_env_sql excxx_example_database_load \
	excxx_example_database_read excxx_lock excxx_mpool \
	excxx_repquote excxx_repquote_gsg_repmgr excxx_repquote_gsg_simple \
	excxx_repquote_gsg_wrfor excxx_sequence excxx_tpcb excxx_order \
	ex_sql_binding ex_sql_fts3 ex_sql_index \
	ex_sql_load ex_sql_multi_thread ex_sql_query ex_sql_rtree \
	ex_sql_savepoint ex_sql_statement ex_sql_transaction exstl_access \
	exstl_advancedfeatures exstl_repquote exstl_tpcb txn_guide test_dbstl \
	test_dbstl_ms_examples test_dbstl_stlport txn_guide_inmemory \
	ex_servicemgmt db_guitest.jar

mostly-clean clean:
	$(RM) -r $(C_OBJS) $(CUTEST_OBJS)
	$(RM) -r $(CXX_OBJS) $(JAVA_OBJS) $(SQL_OBJS) $(SQL_OBJS) $(STL_OBJS)
	$(RM) -r $(TCL_OBJS) $(UTIL_PROGS) *.exe $(CLEAN_LIST)
	$(RM) -r $(JAVA_CLASSTOP) $(JAVA_EXCLASSTOP) $(JAVA_DEMOCLASSTOP) $(JAVA_GUICLASSTOP) $(JAVA_GUITESTCLASSTOP)
	$(RM) -r $(DB_STL_TEST_OBJS) $(TEST_MICRO_OBJS)
	$(RM) -r tags *.lo *.o *.o.lock *.lo core *.core core.*
	$(RM) -r ALL.OUT.* PARALLEL_TESTDIR.*
	$(RM) -r RUN_LOG RUNQUEUE TESTDIR TESTDIR.A TEST.LIST
	$(RM) -r logtrack_seen.db test_micro test_mutex .libs
	$(RM) -r $(LIB_INSTALL_FILE_LIST)
	

REALCLEAN_LIST=\
	Makefile clib_port.h confdefs.h config.cache config.log config.status \
	configure.lineno db.h db185_int.h db_185.h db_config.h db_cxx.h \
	db_int.h db_int_def.h db_provider.c db_provider.h dbstl_common.h \
	db_server.h db_server_clnt.c db_server_svc.c db_server_xdr.c \
	gen_db_server.c include.tcl dbsql.h $(builddir)/sql $(builddir)/jdbc

distclean maintainer-clean realclean: clean
	$(RM) -r $(REALCLEAN_LIST)
	$(RM) -r libtool

check depend dvi info obj TAGS:
	@echo "make: $@ target not available"

dist rpm rpmbuild:
	@echo "make: $@ target not available" && exit 1

##################################################
# Testers, benchmarks.
##################################################
dbs.lo: $(testdir)/server/dbs.c
	$(CC) $(CFLAGS) $?
dbs_am.lo: $(testdir)/server/dbs_am.c
	$(CC) $(CFLAGS) $?
dbs_checkpoint.lo: $(testdir)/server/dbs_checkpoint.c
	$(CC) $(CFLAGS) $?
dbs_debug.lo: $(testdir)/server/dbs_debug.c
	$(CC) $(CFLAGS) $?
dbs_handles.lo: $(testdir)/server/dbs_handles.c
	$(CC) $(CFLAGS) $?
dbs_log.lo: $(testdir)/server/dbs_log.c
	$(CC) $(CFLAGS) $?
dbs_qam.lo: $(testdir)/server/dbs_qam.c
	$(CC) $(CFLAGS) $?
dbs_spawn.lo: $(testdir)/server/dbs_spawn.c
	$(CC) $(CFLAGS) $?
dbs_trickle.lo: $(testdir)/server/dbs_trickle.c
	$(CC) $(CFLAGS) $?
dbs_util.lo: $(testdir)/server/dbs_util.c
	$(CC) $(CFLAGS) $?
dbs_yield.lo: $(testdir)/server/dbs_yield.c
	$(CC) $(CFLAGS) $?
DBS_OBJS=\
	dbs.lo dbs_am.lo dbs_checkpoint.lo dbs_debug.lo dbs_handles.lo \
	dbs_log.lo dbs_qam.lo dbs_spawn.lo dbs_trickle.lo dbs_util.lo \
	dbs_yield.lo
dbs: $(DBS_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(DBS_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

db_perf.lo: $(testdir)/perf/db_perf.c
	$(CC) $(CFLAGS) $?
perf_checkpoint.lo: $(testdir)/perf/perf_checkpoint.c
	$(CC) $(CFLAGS) $?
perf_config.lo: $(testdir)/perf/perf_config.c
	$(CC) $(CFLAGS) $?
perf_dbs.lo: $(testdir)/perf/perf_dbs.c
	$(CC) $(CFLAGS) $?
perf_dead.lo: $(testdir)/perf/perf_dead.c
	$(CC) $(CFLAGS) $?
perf_debug.lo: $(testdir)/perf/perf_debug.c
	$(CC) $(CFLAGS) $?
perf_file.lo: $(testdir)/perf/perf_file.c
	$(CC) $(CFLAGS) $?
perf_key.lo: $(testdir)/perf/perf_key.c
	$(CC) $(CFLAGS) $?
perf_log.lo: $(testdir)/perf/perf_log.c
	$(CC) $(CFLAGS) $?
perf_misc.lo: $(testdir)/perf/perf_misc.c
	$(CC) $(CFLAGS) $?
perf_op.lo: $(testdir)/perf/perf_op.c
	$(CC) $(CFLAGS) $?
perf_parse.lo: $(testdir)/perf/perf_parse.c
	$(CC) $(CFLAGS) $?
perf_rand.lo: $(testdir)/perf/perf_rand.c
	$(CC) $(CFLAGS) $?
perf_spawn.lo: $(testdir)/perf/perf_spawn.c
	$(CC) $(CFLAGS) $?
perf_stat.lo: $(testdir)/perf/perf_stat.c
	$(CC) $(CFLAGS) $?
perf_sync.lo: $(testdir)/perf/perf_sync.c
	$(CC) $(CFLAGS) $?
perf_thread.lo: $(testdir)/perf/perf_thread.c
	$(CC) $(CFLAGS) $?
perf_trickle.lo: $(testdir)/perf/perf_trickle.c
	$(CC) $(CFLAGS) $?
perf_txn.lo: $(testdir)/perf/perf_txn.c
	$(CC) $(CFLAGS) $?
perf_util.lo: $(testdir)/perf/perf_util.c
	$(CC) $(CFLAGS) $?
perf_vx.lo: $(testdir)/perf/perf_vx.c
	$(CC) $(CFLAGS) $?
DBPERF_OBJS=\
	db_perf.lo perf_checkpoint.lo perf_config.lo perf_dbs.lo \
	perf_dead.lo perf_debug.lo perf_file.lo perf_key.lo perf_log.lo \
	perf_misc.lo perf_op.lo perf_parse.lo perf_rand.lo perf_spawn.lo \
	perf_stat.lo perf_sync.lo perf_thread.lo perf_trickle.lo \
	perf_txn.lo perf_util.lo perf_vx.lo util_sig.lo
db_perf: $(DBPERF_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(DBPERF_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

# C unit test suite.
CUTEST_FLAGS= $(CFLAGS) -I$(testdir)/c/cutest -I$(testdir)/c/suites -I$(testdir)/c/common

CuTest.lo: $(testdir)/c/cutest/CuTest.c
	$(CC) $(CUTEST_FLAGS) $?
CuTests.lo: $(testdir)/c/cutest/CuTests.c
	$(CC) $(CUTEST_FLAGS) $?
Runner.lo: $(testdir)/c/cutest/Runner.c
	$(CC) $(CUTEST_FLAGS) $?
TestCallbackSetterAndGetter.lo: $(testdir)/c/suites/TestCallbackSetterAndGetter.c
	$(CC) $(CUTEST_FLAGS) $?
TestChannel.lo: $(testdir)/c/suites/TestChannel.c
	$(CC) $(CUTEST_FLAGS) $?
TestDbHotBackup.lo: $(testdir)/c/suites/TestDbHotBackup.c
	$(CC) $(CUTEST_FLAGS) $?
TestDbTuner.lo: $(testdir)/c/suites/TestDbTuner.c
	$(CC) $(CUTEST_FLAGS) $?
TestEncryption.lo: $(testdir)/c/suites/TestEncryption.c
	$(CC) $(CUTEST_FLAGS) $?
TestEnvConfig.lo: $(testdir)/c/suites/TestEnvConfig.c
	$(CC) $(CUTEST_FLAGS) $?
TestEnvMethod.lo: $(testdir)/c/suites/TestEnvMethod.c
	$(CC) $(CUTEST_FLAGS) $?
TestKeyExistErrorReturn.lo: $(testdir)/c/suites/TestKeyExistErrorReturn.c
	$(CC) $(CUTEST_FLAGS) $?
TestMutexAlignment.lo: $(testdir)/c/suites/TestMutexAlignment.c
	$(CC) $(CUTEST_FLAGS) $?
TestPartial.lo: $(testdir)/c/suites/TestPartial.c
	$(CC) $(CUTEST_FLAGS) $?
TestPartition.lo: $(testdir)/c/suites/TestPartition.c
	$(CC) $(CUTEST_FLAGS) $?
TestPreOpenSetterAndGetter.lo: $(testdir)/c/suites/TestPreOpenSetterAndGetter.c
	$(CC) $(CUTEST_FLAGS) $?
TestQueue.lo: $(testdir)/c/suites/TestQueue.c
	$(CC) $(CUTEST_FLAGS) $?

cutest: $(CUTEST_OBJS) $(DEF_LIB) db_tuner.lo
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(CUTEST_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

TEST_FAILOBJS=test_failchk.lo
test_failchk.lo: $(testdir)/c/test_failchk.c
	$(CC) $(CUTEST_FLAGS) $?
test_failchk: $(TEST_FAILOBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(TEST_FAILOBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)

db_repsite.lo: $(testdir)/repmgr/db_repsite.cpp
	$(CXX) $(CXXFLAGS) $?
DBREPSITE_OBJS=db_repsite.lo
db_repsite: $(DBREPSITE_OBJS) $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ \
	   $(LDFLAGS) $(DBREPSITE_OBJS) $(DEF_LIB_CXX) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@ 

db_reptest.lo: $(testdir)/repmgr/db_reptest.c
	$(CC) $(CFLAGS) $?
reptest_am.lo: $(testdir)/repmgr/reptest_am.c
	$(CC) $(CFLAGS) $?
reptest_handles.lo: $(testdir)/repmgr/reptest_handles.c
	$(CC) $(CFLAGS) $?
reptest_spawn.lo: $(testdir)/repmgr/reptest_spawn.c
	$(CC) $(CFLAGS) $?
reptest_util.lo: $(testdir)/repmgr/reptest_util.c
	$(CC) $(CFLAGS) $?
DBREPTEST_OBJS=\
	db_reptest.lo reptest_am.lo reptest_handles.lo \
	reptest_spawn.lo reptest_util.lo 

db_reptest: $(DBREPTEST_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(DBREPTEST_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

bdb_ha.lo: $(testdir)/repmgr/bdb_ha.c
	$(CC) $(CFLAGS) $?
BDBHA_OBJS=bdb_ha.lo 
bdb_ha: $(BDBHA_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(BDBHA_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

test_dbstl.lo: $(testdir)/stl/base/test_dbstl.cpp
	$(CXX) $(STLFLAGS) $?
test_util.lo: $(testdir)/stl/base/test_util.cpp
	$(CXX) $(STLFLAGS) $?

stl_test.lo: $(testdir)/stl/stlport/stl_test.cpp
	$(CXX) $(STLFLAGS) $?
utility.lo: $(testdir)/stl/stlport/utility.cpp
	$(CXX) $(STLFLAGS) $?
stlport_test_builder.lo: $(testdir)/stl/stlport/stlport_test_builder.cpp
	$(CXX) $(STLFLAGS) $?

ms_stl_main.lo: $(testdir)/stl/ms_examples/ms_stl_main.cpp
	$(CXX) $(STLFLAGS) -DGCC_BAD_AUTO_CONVERSION  $?
ms_stl_common.lo: $(testdir)/stl/ms_examples/ms_stl_common.cpp
	$(CXX) $(STLFLAGS) -DGCC_BAD_AUTO_CONVERSION  $?
test_builder.lo: $(testdir)/stl/ms_examples/test_builder.cpp
	$(CXX) $(STLFLAGS) -DGCC_BAD_AUTO_CONVERSION  $?

DB_STL_TEST_OBJS=test_dbstl.lo test_util.lo
DB_STL_STLPORT_TEST_OBJS=stl_test.lo stlport_test_builder.lo utility.lo
DB_STL_MS_TEST_OBJS=ms_stl_main.lo ms_stl_common.lo \
	test_builder.lo

test_dbstl: $(DB_STL_TEST_OBJS) $(DEF_LIB_CXX) $(DEF_LIB_STL)
	$(CXXLINK) -o $@ $(LDFLAGS) $(DB_STL_TEST_OBJS) \
	    $(DEF_LIB_CXX) $(DEF_LIB_STL) $(LIBS) $(LIBSTLSO_LIBS)
	$(POSTLINK) $@

test_dbstl_ms_examples: $(DB_STL_MS_TEST_OBJS) $(DEF_LIB_CXX) $(DEF_LIB_STL)
	$(CXXLINK) -o $@ $(LDFLAGS) $(DB_STL_MS_TEST_OBJS) \
	    $(DEF_LIB_CXX) $(DEF_LIB_STL) $(LIBS) $(LIBSTLSO_LIBS)
	$(POSTLINK) $@
test_dbstl_stlport: $(DB_STL_STLPORT_TEST_OBJS) $(DEF_LIB_CXX) $(DEF_LIB_STL)
	$(CXXLINK) -o $@ $(LDFLAGS) $(DB_STL_STLPORT_TEST_OBJS) \
	    $(DEF_LIB_CXX) $(DEF_LIB_STL) $(LIBS) $(LIBSTLSO_LIBS)
	$(POSTLINK) $@

b_curalloc.lo: $(testdir)/micro/source/b_curalloc.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_curwalk.lo: $(testdir)/micro/source/b_curwalk.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_del.lo: $(testdir)/micro/source/b_del.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_get.lo: $(testdir)/micro/source/b_get.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_inmem.lo: $(testdir)/micro/source/b_inmem.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_latch.lo: $(testdir)/micro/source/b_latch.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_load.lo: $(testdir)/micro/source/b_load.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_open.lo: $(testdir)/micro/source/b_open.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_put.lo: $(testdir)/micro/source/b_put.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_recover.lo: $(testdir)/micro/source/b_recover.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_txn.lo: $(testdir)/micro/source/b_txn.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_txn_write.lo: $(testdir)/micro/source/b_txn_write.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_uname.lo: $(testdir)/micro/source/b_uname.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_util.lo: $(testdir)/micro/source/b_util.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
b_workload.lo: $(testdir)/micro/source/b_workload.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
test_micro.lo: $(testdir)/micro/source/test_micro.c
	$(CC) $(CFLAGS) -I$(testdir)/micro/source $?
test_micro: $(TEST_MICRO_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(TEST_MICRO_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

test_mutex.lo: $(srcdir)/mutex/test_mutex.c
	$(CC) $(CFLAGS) $?
test_mutex: test_mutex.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) test_mutex.lo $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

##################################################
# Targets for example programs.
##################################################
examples_c: ex_access ex_apprec \
	ex_btrec ex_bulk ex_env ex_external_file ex_firewall \
	ex_firewall_dest ex_firewall_msg ex_heap ex_lock \
	ex_mpool ex_priority ex_priority_dest ex_priority_msg ex_rep_base \
	ex_rep_chan ex_rep_gsg_wrfor ex_rep_gsg_repmgr ex_rep_gsg_simple \
	ex_rep_mgr ex_sequence ex_slice ex_stream ex_thread ex_tpcb \
	example_database_load example_database_read txn_guide \
	txn_guide_inmemory ex_servicemgmt

examples_cxx: TxnGuide TxnGuideInMemory excxx_access excxx_btrec excxx_bulk \
	excxx_env excxx_env_sql excxx_example_database_load excxx_example_database_read \
	excxx_lock excxx_mpool excxx_repquote excxx_repquote_gsg_repmgr \
	excxx_repquote_gsg_simple excxx_repquote_gsg_wrfor \
	excxx_sequence excxx_tpcb excxx_order

examples_stl: StlTxnGuide exstl_access exstl_advancedfeatures exstl_repquote \
	exstl_tpcb

examples_sql: ex_advertising ex_ad_data ex_ad_event ex_sql_binding \
	ex_sql_fts3 ex_sql_index ex_sql_load \
	ex_sql_multi_thread ex_sql_query ex_sql_rtree ex_sql_savepoint \
	ex_sql_statement ex_sql_transaction ex_toll_booth ex_toll_data \
	ex_toll_event

examples: examples_c examples_cxx examples_stl examples_sql

##################################################
# Example programs for C.
##################################################
ex_access.lo: $(exampledir)/c/ex_access.c
	$(CC) $(CFLAGS) $?
ex_access: ex_access.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_access.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_apprec.lo: $(exampledir)/c/ex_apprec/ex_apprec.c
	$(CC) $(CFLAGS) $?
ex_apprec_auto.lo: $(exampledir)/c/ex_apprec/ex_apprec_auto.c
	$(CC) $(CFLAGS) $?
ex_apprec_autop.lo: $(exampledir)/c/ex_apprec/ex_apprec_autop.c
	$(CC) $(CFLAGS) $?
ex_apprec_rec.lo: $(exampledir)/c/ex_apprec/ex_apprec_rec.c
	$(CC) $(CFLAGS) $?
EX_APPREC_OBJS=\
	ex_apprec.lo ex_apprec_auto.lo ex_apprec_autop.lo ex_apprec_rec.lo
ex_apprec: $(EX_APPREC_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(EX_APPREC_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)

ex_external_file.lo: $(exampledir)/c/ex_external_file.c
	$(CC) $(CFLAGS) $?
ex_external_file: ex_external_file.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_external_file.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_btrec.lo: $(exampledir)/c/ex_btrec.c
	$(CC) $(CFLAGS) $?
ex_btrec: ex_btrec.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_btrec.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_bulk.lo: $(exampledir)/c/ex_bulk.c
	$(CC) $(CFLAGS) $?
ex_bulk: ex_bulk.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_bulk.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_dbclient.lo: $(exampledir)/c/ex_dbclient.c
	$(CC) $(CFLAGS) $?
ex_dbclient: ex_dbclient.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_dbclient.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_env.lo: $(exampledir)/c/ex_env.c
	$(CC) $(CFLAGS) $?
ex_env: ex_env.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_env.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

servicemgmt.lo: $(exampledir)/c/ex_servicemgmt/servicemgmt.c
	$(CC) $(CFLAGS) $?
servicemgmt_dboperation.lo: $(exampledir)/c/ex_servicemgmt/servicemgmt_dboperation.c
	$(CC) $(CFLAGS) $?
servicemgmt_login.lo: $(exampledir)/c/ex_servicemgmt/servicemgmt_login.c
	$(CC) $(CFLAGS) $?
servicemgmt_setup.lo: $(exampledir)/c/ex_servicemgmt/servicemgmt_setup.c
	$(CC) $(CFLAGS) $?
SERVICEMGMT_OBJS=\
	servicemgmt.lo servicemgmt_dboperation.lo servicemgmt_login.lo servicemgmt_setup.lo
ex_servicemgmt: $(SERVICEMGMT_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(SERVICEMGMT_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

firewall.lo: $(exampledir)/c/ex_firewall/firewall.c
	$(CC) $(CFLAGS) $?
firewall_bdb_filters.lo: $(exampledir)/c/ex_firewall/firewall_bdb_filters.c
	$(CC) $(CFLAGS) $?
firewall_common_utils.lo: $(exampledir)/c/ex_firewall/firewall_common_utils.c
	$(CC) $(CFLAGS) $?
FIREWALL_OBJS=\
	firewall.lo firewall_bdb_filters.lo firewall_common_utils.lo
ex_firewall: $(FIREWALL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(FIREWALL_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

firewall_destination.lo: $(exampledir)/c/ex_firewall/firewall_destination.c
	$(CC) $(CFLAGS) $?
FIREWALL_DEST_OBJS= firewall_common_utils.lo firewall_destination.lo
ex_firewall_dest: $(FIREWALL_DEST_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(FIREWALL_DEST_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

firewall_message_generator.lo: $(exampledir)/c/ex_firewall/firewall_message_generator.c
	$(CC) $(CFLAGS) $?
FIREWALL_MSG_OBJS= firewall_common_utils.lo firewall_message_generator.lo
ex_firewall_msg: $(FIREWALL_MSG_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(FIREWALL_MSG_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

ex_heap.lo: $(exampledir)/c/ex_heap.c
	$(CC) $(CFLAGS) $?
ex_heap: ex_heap.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_heap.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_lock.lo: $(exampledir)/c/ex_lock.c
	$(CC) $(CFLAGS) $?
ex_lock: ex_lock.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_lock.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_mpool.lo: $(exampledir)/c/ex_mpool.c
	$(CC) $(CFLAGS) $?
ex_mpool: ex_mpool.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_mpool.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

priority_sorter.lo: $(exampledir)/c/ex_priority/priority_sorter.c
	$(CC) $(CFLAGS) $?
priority_bdb_filters.lo: $(exampledir)/c/ex_priority/priority_bdb_filters.c
	$(CC) $(CFLAGS) $?
PRIORITY_OBJS=\
	priority_sorter.lo priority_bdb_filters.lo priority_common_utils.lo
ex_priority: $(PRIORITY_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(PRIORITY_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

priority_common_utils.lo: $(exampledir)/c/ex_priority/priority_common_utils.c
	$(CC) $(CFLAGS) $?
priority_destination.lo: $(exampledir)/c/ex_priority/priority_destination.c
	$(CC) $(CFLAGS) $?
PRIORITY_DEST_OBJS= priority_common_utils.lo priority_destination.lo
ex_priority_dest: $(PRIORITY_DEST_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(PRIORITY_DEST_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

priority_message_generator.lo: $(exampledir)/c/ex_priority/priority_message_generator.c
	$(CC) $(CFLAGS) $?
PRIORITY_MSG_OBJS= priority_common_utils.lo priority_message_generator.lo
ex_priority_msg: $(PRIORITY_MSG_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(PRIORITY_MSG_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

rep_base.lo: $(exampledir)/c/ex_rep/base/rep_base.c
	$(CC) $(CFLAGS) $?
rep_common.lo: $(exampledir)/c/ex_rep/common/rep_common.c
	$(CC) $(CFLAGS) $?
rep_msg.lo: $(exampledir)/c/ex_rep/base/rep_msg.c
	$(CC) $(CFLAGS) $?
rep_net.lo: $(exampledir)/c/ex_rep/base/rep_net.c
	$(CC) $(CFLAGS) $?
EX_REP_BASE_OBJS=\
	rep_base.lo rep_common.lo rep_msg.lo rep_net.lo
ex_rep_base: $(EX_REP_BASE_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(EX_REP_BASE_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

rep_chan.lo: $(exampledir)/c/ex_rep_chan/rep_chan.c
	$(CC) $(CFLAGS) $?
rep_chan_util.lo: $(exampledir)/c/ex_rep_chan/rep_chan_util.c
	$(CC) $(CFLAGS) $?
EX_REP_CHAN_OBJS=\
	rep_chan.lo rep_chan_util.lo
ex_rep_chan: $(EX_REP_CHAN_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(EX_REP_CHAN_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

simple_txn.lo: $(exampledir)/c/ex_rep_gsg/simple_txn.c
	$(CC) $(CFLAGS) $?
ex_rep_gsg_simple: simple_txn.lo $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) simple_txn.lo $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

rep_mgr_gsg.lo: $(exampledir)/c/ex_rep_gsg/rep_mgr_gsg.c
	$(CC) $(CFLAGS) $?
ex_rep_gsg_repmgr: rep_mgr_gsg.lo $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) rep_mgr_gsg.lo $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

rep_mgr_wrfor_gsg.lo: $(exampledir)/c/ex_rep_gsg/rep_mgr_wrfor_gsg.c
	$(CC) $(CFLAGS) $?
ex_rep_gsg_wrfor: rep_mgr_wrfor_gsg.lo $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) rep_mgr_wrfor_gsg.lo $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

rep_mgr.lo: $(exampledir)/c/ex_rep/mgr/rep_mgr.c
	$(CC) $(CFLAGS) $?
EX_REP_MGR_OBJS=\
	rep_common.lo rep_mgr.lo
ex_rep_mgr: $(EX_REP_MGR_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(EX_REP_MGR_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

ex_sequence.lo: $(exampledir)/c/ex_sequence.c
	$(CC) $(CFLAGS) $?
ex_sequence: ex_sequence.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_sequence.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_slice.lo: $(exampledir)/c/ex_slice.c
	$(CC) $(CFLAGS) $?
ex_slice: ex_slice.lo $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) ex_slice.lo $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

ex_stream.lo: $(exampledir)/c/ex_stream.c
	$(CC) $(CFLAGS) $?
ex_stream: ex_stream.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_stream.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_thread.lo: $(exampledir)/c/ex_thread.c
	$(CC) $(CFLAGS) $?
ex_thread: ex_thread.lo $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) ex_thread.lo $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

ex_tpcb.lo: $(exampledir)/c/ex_tpcb.c
	$(CC) $(CFLAGS) $?
ex_tpcb: ex_tpcb.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_tpcb.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

gettingstarted_common.lo: \
    $(exampledir)/c/getting_started/gettingstarted_common.c
	$(CC) -I$(exampledir)/c/getting_started $(CFLAGS) $?
example_database_load.lo: \
    $(exampledir)/c/getting_started/example_database_load.c
	$(CC) $(CFLAGS) $?
example_database_read.lo: \
    $(exampledir)/c/getting_started/example_database_read.c
	$(CC) $(CFLAGS) $?
example_database_load: example_database_load.lo gettingstarted_common.lo \
    $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    example_database_load.lo gettingstarted_common.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@
example_database_read: example_database_read.lo gettingstarted_common.lo \
    $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    example_database_read.lo gettingstarted_common.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

txn_guide.lo: $(exampledir)/c/txn_guide/txn_guide.c
	$(CC) $(CFLAGS) $?
txn_guide: txn_guide.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) txn_guide.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

txn_guide_inmemory.lo: $(exampledir)/c/txn_guide/txn_guide_inmemory.c
	$(CC) $(CFLAGS) $?
txn_guide_inmemory: txn_guide_inmemory.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) txn_guide_inmemory.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

##################################################
# Example programs for C++.
##################################################
AccessExample.lo: $(exampledir)/cxx/AccessExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_access: AccessExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) AccessExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

BtRecExample.lo: $(exampledir)/cxx/BtRecExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_btrec: BtRecExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) BtRecExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

BulkExample.lo: $(exampledir)/cxx/BulkExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_bulk: BulkExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) BulkExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

EnvExample.lo: $(exampledir)/cxx/EnvExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_env: EnvExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) EnvExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

EnvLoadSqlExample.lo: $(exampledir)/cxx/EnvLoadSqlExample.cpp
	$(CXX) $(CXXFLAGS) $(SQLFLAGS) $?
excxx_env_sql: EnvLoadSqlExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) EnvLoadSqlExample.lo $(DEF_LIB_CXX) $(DEF_LIB_SQL) $(LIBS)
	$(POSTLINK) $@

LockExample.lo: $(exampledir)/cxx/LockExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_lock: LockExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) LockExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

MpoolExample.lo: $(exampledir)/cxx/MpoolExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_mpool: MpoolExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) MpoolExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

RepQuoteExample.lo: $(exampledir)/cxx/excxx_repquote/RepQuoteExample.cpp
	$(CXX) -I$(exampledir)/cxx/excxx_repquote $(CXXFLAGS) $?
RepConfigInfo.lo: $(exampledir)/cxx/excxx_repquote/RepConfigInfo.cpp
	$(CXX) -I$(exampledir)/cxx/excxx_repquote $(CXXFLAGS) $?
excxx_repquote: RepQuoteExample.lo RepConfigInfo.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) \
	    RepQuoteExample.lo RepConfigInfo.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

OrderSystem.lo: $(exampledir)/cxx/excxx_order/OrderSystem.cpp
	$(CXX) -I$(exampledir)/cxx/excxx_order $(CXXFLAGS) $?
excxx_order: OrderSystem.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) \
	    OrderSystem.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

RepMgrGSG.lo: $(exampledir)/cxx/excxx_repquote_gsg/RepMgrGSG.cpp
	$(CXX) -I$(exampledir)/cxx/excxx_repquote_gsg $(CXXFLAGS) $?
excxx_repquote_gsg_repmgr: RepMgrGSG.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) RepMgrGSG.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

RepMgrWrforGSG.lo: $(exampledir)/cxx/excxx_repquote_gsg/RepMgrWrforGSG.cpp
	$(CXX) -I$(exampledir)/cxx/excxx_repquote_gsg $(CXXFLAGS) $?
excxx_repquote_gsg_wrfor: RepMgrWrforGSG.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) RepMgrWrforGSG.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

SimpleTxn.lo: $(exampledir)/cxx/excxx_repquote_gsg/SimpleTxn.cpp
	$(CXX) -I$(exampledir)/cxx/excxx_repquote_gsg $(CXXFLAGS) $?
excxx_repquote_gsg_simple: SimpleTxn.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) SimpleTxn.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

SequenceExample.lo: $(exampledir)/cxx/SequenceExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_sequence: SequenceExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) SequenceExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

TpcbExample.lo: $(exampledir)/cxx/TpcbExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_tpcb: TpcbExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) TpcbExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

excxx_example_database_load.lo: \
    $(exampledir)/cxx/getting_started/excxx_example_database_load.cpp
	$(CXX) -I$(exampledir)/cxx/getting_started $(CXXFLAGS) $?
excxx_example_database_read.lo: \
    $(exampledir)/cxx/getting_started/excxx_example_database_read.cpp
	$(CXX) -I$(exampledir)/cxx/getting_started $(CXXFLAGS) $?
MyDb.lo: $(exampledir)/cxx/getting_started/MyDb.cpp
	$(CXX) -I$(exampledir)/cxx/getting_started $(CXXFLAGS) $?
excxx_example_database_load: \
    excxx_example_database_load.lo MyDb.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) \
	    excxx_example_database_load.lo MyDb.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@
excxx_example_database_read: \
    excxx_example_database_read.lo MyDb.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) \
	    excxx_example_database_read.lo MyDb.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

TxnGuideInMemory.lo: $(exampledir)/cxx/txn_guide/TxnGuideInMemory.cpp
	$(CXX) $(CXXFLAGS) $?
TxnGuideInMemory: TxnGuideInMemory.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) TxnGuideInMemory.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

TxnGuide.lo: $(exampledir)/cxx/txn_guide/TxnGuide.cpp
	$(CXX) $(CXXFLAGS) $?
TxnGuide: TxnGuide.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) TxnGuide.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

##################################################
# Example programs for STL.
##################################################
StlAccessExample.lo: $(exampledir)/stl/StlAccessExample.cpp
	$(CXX) $(STLFLAGS) $?
exstl_access: StlAccessExample.lo $(DEF_LIB_STL)
	$(CXXLINK) -o $@ $(LDFLAGS) StlAccessExample.lo $(DEF_LIB_STL) $(LIBS)
	$(POSTLINK) $@

StlAdvancedFeatures.lo: $(exampledir)/stl/StlAdvancedFeatures.cpp
	$(CXX) -I$(exampledir)/stl $(STLFLAGS) $?
exstl_advancedfeatures: StlAdvancedFeatures.lo $(DEF_LIB_STL)
	$(CXXLINK) -o $@ $(LDFLAGS) StlAdvancedFeatures.lo $(DEF_LIB_STL) $(LIBS)
	$(POSTLINK) $@

StlRepQuoteExample.lo: $(exampledir)/stl/repquote/StlRepQuoteExample.cpp
	$(CXX) -I$(exampledir)/stl/repquote $(STLFLAGS) $?
StlRepConfigInfo.lo: $(exampledir)/stl/repquote/StlRepConfigInfo.cpp
	$(CXX) -I$(exampledir)/stl/repquote $(STLFLAGS) $?
exstl_repquote: StlRepQuoteExample.lo StlRepConfigInfo.lo $(DEF_LIB_STL)
	$(CXXLINK) -o $@ $(LDFLAGS) \
	    StlRepQuoteExample.lo StlRepConfigInfo.lo $(DEF_LIB_STL) $(LIBS)
	$(POSTLINK) $@

StlTpcbExample.lo: $(exampledir)/stl/StlTpcbExample.cpp
	$(CXX) $(STLFLAGS) $?
exstl_tpcb: StlTpcbExample.lo $(DEF_LIB_STL)
	$(CXXLINK) -o $@ $(LDFLAGS) StlTpcbExample.lo $(DEF_LIB_STL) $(LIBS)
	$(POSTLINK) $@

StlTransactionGuideExample.lo: $(exampledir)/stl/StlTransactionGuideExample.cpp
	$(CXX) $(STLFLAGS) $?
StlTxnGuide: StlTransactionGuideExample.lo $(DEF_LIB_STL)
	$(CXXLINK) -o $@ $(LDFLAGS) StlTransactionGuideExample.lo $(DEF_LIB_STL) $(LIBS)
	$(POSTLINK) $@

##################################################
# Example programs for SQL.
##################################################
ex_advertising: advertise.lo ad_bdb_filters.lo ad_common_utils.lo ad_event_processor.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) advertise.lo ad_event_processor.lo ad_bdb_filters.lo ad_common_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_ad_data: ad_data_generator.lo ad_common_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) ad_data_generator.lo ad_common_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_ad_event: ad_event_generator.lo ad_common_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) ad_event_generator.lo ad_common_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_sql_binding: ex_sql_binding.lo ex_sql_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) ex_sql_binding.lo ex_sql_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_sql_fts3: ex_sql_fts3.lo ex_sql_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) ex_sql_fts3.lo ex_sql_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_sql_index: ex_sql_index.lo ex_sql_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) ex_sql_index.lo ex_sql_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_sql_load: ex_sql_load.lo ex_sql_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) ex_sql_load.lo ex_sql_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_sql_multi_thread: ex_sql_multi_thread.lo ex_sql_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) ex_sql_multi_thread.lo ex_sql_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_sql_query: ex_sql_query.lo ex_sql_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) ex_sql_query.lo ex_sql_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_sql_rtree: ex_sql_rtree.lo ex_sql_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) ex_sql_rtree.lo ex_sql_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_sql_savepoint: ex_sql_savepoint.lo ex_sql_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) ex_sql_savepoint.lo ex_sql_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_sql_statement: ex_sql_statement.lo ex_sql_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) ex_sql_statement.lo ex_sql_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_sql_transaction: ex_sql_transaction.lo ex_sql_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) ex_sql_transaction.lo ex_sql_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_toll_booth: billing.lo toll_bdb_filters.lo toll_common_utils.lo toll_event_processor.lo stolen.lo traffic.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) billing.lo toll_bdb_filters.lo toll_common_utils.lo toll_event_processor.lo stolen.lo traffic.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_toll_data: toll_data_generator.lo toll_common_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) toll_data_generator.lo toll_common_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

ex_toll_event: toll_event_generator.lo toll_common_utils.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) toll_event_generator.lo toll_common_utils.lo $(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

advertise.lo: $(exampledir)/sql/ex_advertising/advertise.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ad_bdb_filters.lo: $(exampledir)/sql/ex_advertising/ad_bdb_filters.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ad_common_utils.lo: $(exampledir)/sql/ex_advertising/ad_common_utils.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ad_data_generator.lo: $(exampledir)/sql/ex_advertising/ad_data_generator.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ad_event_generator.lo: $(exampledir)/sql/ex_advertising/ad_event_generator.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ad_event_processor.lo: $(exampledir)/sql/ex_advertising/ad_event_processor.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
billing.lo: $(exampledir)/sql/ex_toll_booth/billing.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ex_sql_binding.lo: $(exampledir)/sql/c/ex_sql_binding.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ex_sql_fts3.lo: $(exampledir)/sql/c/ex_sql_fts3.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ex_sql_index.lo: $(exampledir)/sql/c/ex_sql_index.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ex_sql_load.lo: $(exampledir)/sql/c/ex_sql_load.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ex_sql_multi_thread.lo: $(exampledir)/sql/c/ex_sql_multi_thread.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ex_sql_utils.lo: $(exampledir)/sql/c/ex_sql_utils.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ex_sql_query.lo: $(exampledir)/sql/c/ex_sql_query.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ex_sql_rtree.lo: $(exampledir)/sql/c/ex_sql_rtree.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ex_sql_savepoint.lo: $(exampledir)/sql/c/ex_sql_savepoint.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ex_sql_statement.lo: $(exampledir)/sql/c/ex_sql_statement.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
ex_sql_transaction.lo: $(exampledir)/sql/c/ex_sql_transaction.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
stolen.lo: $(exampledir)/sql/ex_toll_booth/stolen.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
toll_bdb_filters.lo: $(exampledir)/sql/ex_toll_booth/toll_bdb_filters.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
toll_common_utils.lo: $(exampledir)/sql/ex_toll_booth/toll_common_utils.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
toll_data_generator.lo: $(exampledir)/sql/ex_toll_booth/toll_data_generator.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
toll_event_generator.lo: $(exampledir)/sql/ex_toll_booth/toll_event_generator.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
toll_event_processor.lo: $(exampledir)/sql/ex_toll_booth/toll_event_processor.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
traffic.lo: $(exampledir)/sql/ex_toll_booth/traffic.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?

##################################################
# Test programs for SQL.
##################################################
bdb_sqlthreadtest.lo: $(testdir)/sql/bdb_sqlthreadtest.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?

bdb_sqlthreadtest: bdb_sqlthreadtest.lo $(DEF_LIB_SQL)
	$(CCLINK) -o $@ $(LDFLAGS) bdb_sqlthreadtest.lo \
		$(DEF_LIB_SQL) @SQL_LIBS@ $(LIBS)
	$(POSTLINK) $@

##################################################
# C API build rules.
##################################################
aes_method.lo: $(srcdir)/crypto/aes_method.c
	 $(CC) $(CFLAGS) $?
blob_fileops.lo: $(srcdir)/blob/blob_fileops.c
	 $(CC) $(CFLAGS) $?
blob_page.lo: $(srcdir)/blob/blob_page.c
	 $(CC) $(CFLAGS) $?
blob_stream.lo: $(srcdir)/blob/blob_stream.c
	 $(CC) $(CFLAGS) $?
blob_util.lo: $(srcdir)/blob/blob_util.c
	 $(CC) $(CFLAGS) $?
bt_compare.lo: $(srcdir)/btree/bt_compare.c
	 $(CC) $(CFLAGS) $?
bt_compress.lo: $(srcdir)/btree/bt_compress.c
	 $(CC) $(CFLAGS) $?
bt_conv.lo: $(srcdir)/btree/bt_conv.c
	 $(CC) $(CFLAGS) $?
bt_curadj.lo: $(srcdir)/btree/bt_curadj.c
	 $(CC) $(CFLAGS) $?
bt_cursor.lo: $(srcdir)/btree/bt_cursor.c
	 $(CC) $(CFLAGS) $?
bt_delete.lo: $(srcdir)/btree/bt_delete.c
	 $(CC) $(CFLAGS) $?
bt_method.lo: $(srcdir)/btree/bt_method.c
	 $(CC) $(CFLAGS) $?
bt_open.lo: $(srcdir)/btree/bt_open.c
	 $(CC) $(CFLAGS) $?
bt_put.lo: $(srcdir)/btree/bt_put.c
	 $(CC) $(CFLAGS) $?
bt_rec.lo: $(srcdir)/btree/bt_rec.c
	 $(CC) $(CFLAGS) $?
bt_reclaim.lo: $(srcdir)/btree/bt_reclaim.c
	 $(CC) $(CFLAGS) $?
bt_recno.lo: $(srcdir)/btree/bt_recno.c
	 $(CC) $(CFLAGS) $?
bt_rsearch.lo: $(srcdir)/btree/bt_rsearch.c
	 $(CC) $(CFLAGS) $?
bt_search.lo: $(srcdir)/btree/bt_search.c
	 $(CC) $(CFLAGS) $?
bt_split.lo: $(srcdir)/btree/bt_split.c
	 $(CC) $(CFLAGS) $?
bt_stat.lo: $(srcdir)/btree/bt_stat.c
	 $(CC) $(CFLAGS) $?
bt_compact.lo: $(srcdir)/btree/bt_compact.c
	 $(CC) $(CFLAGS) $?
bt_upgrade.lo: $(srcdir)/btree/bt_upgrade.c
	 $(CC) $(CFLAGS) $?
bt_verify.lo: $(srcdir)/btree/bt_verify.c
	 $(CC) $(CFLAGS) $?
btree_auto.lo: $(srcdir)/btree/btree_auto.c
	 $(CC) $(CFLAGS) $?
btree_autop.lo: $(srcdir)/btree/btree_autop.c
	 $(CC) $(CFLAGS) $?
clock.lo: $(srcdir)/common/clock.c
	 $(CC) $(CFLAGS) $?
crdel_auto.lo: $(srcdir)/db/crdel_auto.c
	 $(CC) $(CFLAGS) $?
crdel_autop.lo: $(srcdir)/db/crdel_autop.c
	 $(CC) $(CFLAGS) $?
crdel_rec.lo: $(srcdir)/db/crdel_rec.c
	 $(CC) $(CFLAGS) $?
crypto.lo: $(srcdir)/crypto/crypto.c
	 $(CC) $(CFLAGS) $?
crypto_stub.lo: $(srcdir)/common/crypto_stub.c
	 $(CC) $(CFLAGS) $?
db185.lo: $(langdir)/db185/db185.c
	 $(CC) $(CFLAGS) $?
db.lo: $(srcdir)/db/db.c
	 $(CC) $(CFLAGS) $?
db_am.lo: $(srcdir)/db/db_am.c
	 $(CC) $(CFLAGS) $?
db_auto.lo: $(srcdir)/db/db_auto.c
	 $(CC) $(CFLAGS) $?
db_autop.lo: $(srcdir)/db/db_autop.c
	 $(CC) $(CFLAGS) $?
db_byteorder.lo: $(srcdir)/common/db_byteorder.c
	 $(CC) $(CFLAGS) $?
db_backup.lo: $(srcdir)/db/db_backup.c
	 $(CC) $(CFLAGS) $?
db_cam.lo: $(srcdir)/db/db_cam.c
	 $(CC) $(CFLAGS) $?
db_cds.lo: $(srcdir)/db/db_cds.c
	 $(CC) $(CFLAGS) $?
db_compact.lo: $(srcdir)/db/db_compact.c
	 $(CC) $(CFLAGS) $?
db_compint.lo: $(srcdir)/common/db_compint.c
	 $(CC) $(CFLAGS) $?
db_conv.lo: $(srcdir)/db/db_conv.c
	 $(CC) $(CFLAGS) $?
db_copy.lo: $(srcdir)/db/db_copy.c
	$(CC) $(CFLAGS) $?
db_dispatch.lo: $(srcdir)/db/db_dispatch.c
	 $(CC) $(CFLAGS) $?
db_dup.lo: $(srcdir)/db/db_dup.c
	 $(CC) $(CFLAGS) $?
db_err.lo: $(srcdir)/common/db_err.c
	 $(CC) $(CFLAGS) $?
db_getlong.lo: $(srcdir)/common/db_getlong.c
	 $(CC) $(CFLAGS) $?
db_idspace.lo: $(srcdir)/common/db_idspace.c
	 $(CC) $(CFLAGS) $?
db_iface.lo: $(srcdir)/db/db_iface.c
	 $(CC) $(CFLAGS) $?
db_join.lo: $(srcdir)/db/db_join.c
	 $(CC) $(CFLAGS) $?
db_log2.lo: $(srcdir)/common/db_log2.c
	 $(CC) $(CFLAGS) $?
db_meta.lo: $(srcdir)/db/db_meta.c
	 $(CC) $(CFLAGS) $?
db_method.lo: $(srcdir)/db/db_method.c
	 $(CC) $(CFLAGS) $?
db_open.lo: $(srcdir)/db/db_open.c
	 $(CC) $(CFLAGS) $?
db_overflow.lo: $(srcdir)/db/db_overflow.c
	 $(CC) $(CFLAGS) $?
db_ovfl_vrfy.lo: $(srcdir)/db/db_ovfl_vrfy.c
	 $(CC) $(CFLAGS) $?
db_pr.lo: $(srcdir)/db/db_pr.c
	 $(CC) $(CFLAGS) $?
db_rec.lo: $(srcdir)/db/db_rec.c
	 $(CC) $(CFLAGS) $?
db_reclaim.lo: $(srcdir)/db/db_reclaim.c
	 $(CC) $(CFLAGS) $?
db_rename.lo: $(srcdir)/db/db_rename.c
	 $(CC) $(CFLAGS) $?
db_remove.lo: $(srcdir)/db/db_remove.c
	 $(CC) $(CFLAGS) $?
db_ret.lo: $(srcdir)/db/db_ret.c
	 $(CC) $(CFLAGS) $?
db_setid.lo: $(srcdir)/db/db_setid.c
	 $(CC) $(CFLAGS) $?
db_setlsn.lo: $(srcdir)/db/db_setlsn.c
	 $(CC) $(CFLAGS) $?
db_slice.lo: $(srcdir)/db/db_slice.c
	 $(CC) $(CFLAGS) $?
db_shash.lo: $(srcdir)/common/db_shash.c
	 $(CC) $(CFLAGS) $?
db_sort_multiple.lo: $(srcdir)/db/db_sort_multiple.c
	 $(CC) $(CFLAGS) $?
db_stati.lo: $(srcdir)/db/db_stati.c
	 $(CC) $(CFLAGS) $?
db_truncate.lo: $(srcdir)/db/db_truncate.c
	 $(CC) $(CFLAGS) $?
db_upg.lo: $(srcdir)/db/db_upg.c
	 $(CC) $(CFLAGS) $?
db_upg_opd.lo: $(srcdir)/db/db_upg_opd.c
	 $(CC) $(CFLAGS) $?
db_vrfy.lo: $(srcdir)/db/db_vrfy.c
	 $(CC) $(CFLAGS) $?
db_vrfyutil.lo: $(srcdir)/db/db_vrfyutil.c
	 $(CC) $(CFLAGS) $?
db_vrfy_stub.lo: $(srcdir)/db/db_vrfy_stub.c
	$(CC) $(CFLAGS) $?
dbm.lo: $(langdir)/dbm/dbm.c
	 $(CC) $(CFLAGS) $?
dbreg.lo: $(srcdir)/dbreg/dbreg.c
	 $(CC) $(CFLAGS) $?
dbreg_auto.lo: $(srcdir)/dbreg/dbreg_auto.c
	 $(CC) $(CFLAGS) $?
dbreg_autop.lo: $(srcdir)/dbreg/dbreg_autop.c
	 $(CC) $(CFLAGS) $?
dbreg_rec.lo: $(srcdir)/dbreg/dbreg_rec.c
	 $(CC) $(CFLAGS) $?
dbreg_stat.lo: $(srcdir)/dbreg/dbreg_stat.c
	 $(CC) $(CFLAGS) $?
dbreg_util.lo: $(srcdir)/dbreg/dbreg_util.c
	 $(CC) $(CFLAGS) $?
dbt.lo: $(srcdir)/common/dbt.c
	 $(CC) $(CFLAGS) $?
env_alloc.lo: $(srcdir)/env/env_alloc.c
	 $(CC) $(CFLAGS) $?
env_config.lo: $(srcdir)/env/env_config.c
	 $(CC) $(CFLAGS) $?
env_backup.lo: $(srcdir)/env/env_backup.c
	 $(CC) $(CFLAGS) $?
env_failchk.lo: $(srcdir)/env/env_failchk.c
	 $(CC) $(CFLAGS) $?
env_file.lo: $(srcdir)/env/env_file.c
	 $(CC) $(CFLAGS) $?
env_globals.lo: $(srcdir)/env/env_globals.c
	 $(CC) $(CFLAGS) $?
env_method.lo: $(srcdir)/env/env_method.c
	 $(CC) $(CFLAGS) $?
env_name.lo: $(srcdir)/env/env_name.c
	 $(CC) $(CFLAGS) $?
env_open.lo: $(srcdir)/env/env_open.c
	 $(CC) $(CFLAGS) $?
env_recover.lo: $(srcdir)/env/env_recover.c
	 $(CC) $(CFLAGS) $?
env_region.lo: $(srcdir)/env/env_region.c
	 $(CC) $(CFLAGS) $?
env_register.lo: $(srcdir)/env/env_register.c
	 $(CC) $(CFLAGS) $?
env_sig.lo: $(srcdir)/env/env_sig.c
	 $(CC) $(CFLAGS) $?
env_slice.lo: $(srcdir)/env/env_slice.c
	 $(CC) $(CFLAGS) $?
env_stat.lo: $(srcdir)/env/env_stat.c
	 $(CC) $(CFLAGS) $?
fileops_auto.lo: $(srcdir)/fileops/fileops_auto.c
	 $(CC) $(CFLAGS) $?
fileops_autop.lo: $(srcdir)/fileops/fileops_autop.c
	 $(CC) $(CFLAGS) $?
fop_basic.lo: $(srcdir)/fileops/fop_basic.c
	 $(CC) $(CFLAGS) $?
fop_rec.lo: $(srcdir)/fileops/fop_rec.c
	 $(CC) $(CFLAGS) $?
fop_util.lo: $(srcdir)/fileops/fop_util.c
	 $(CC) $(CFLAGS) $?
hash.lo: $(srcdir)/hash/hash.c
	 $(CC) $(CFLAGS) $?
hash_auto.lo: $(srcdir)/hash/hash_auto.c
	 $(CC) $(CFLAGS) $?
hash_autop.lo: $(srcdir)/hash/hash_autop.c
	 $(CC) $(CFLAGS) $?
hash_compact.lo: $(srcdir)/hash/hash_compact.c
	 $(CC) $(CFLAGS) $?
hash_conv.lo: $(srcdir)/hash/hash_conv.c
	 $(CC) $(CFLAGS) $?
hash_dup.lo: $(srcdir)/hash/hash_dup.c
	 $(CC) $(CFLAGS) $?
hash_func.lo: $(srcdir)/hash/hash_func.c
	 $(CC) $(CFLAGS) $?
hash_meta.lo: $(srcdir)/hash/hash_meta.c
	 $(CC) $(CFLAGS) $?
hash_method.lo: $(srcdir)/hash/hash_method.c
	 $(CC) $(CFLAGS) $?
hash_open.lo: $(srcdir)/hash/hash_open.c
	 $(CC) $(CFLAGS) $?
hash_page.lo: $(srcdir)/hash/hash_page.c
	 $(CC) $(CFLAGS) $?
hash_rec.lo: $(srcdir)/hash/hash_rec.c
	 $(CC) $(CFLAGS) $?
hash_reclaim.lo: $(srcdir)/hash/hash_reclaim.c
	 $(CC) $(CFLAGS) $?
hash_stat.lo: $(srcdir)/hash/hash_stat.c
	 $(CC) $(CFLAGS) $?
hash_stub.lo: $(srcdir)/hash/hash_stub.c
	 $(CC) $(CFLAGS) $?
hash_upgrade.lo: $(srcdir)/hash/hash_upgrade.c
	 $(CC) $(CFLAGS) $?
hash_verify.lo: $(srcdir)/hash/hash_verify.c
	 $(CC) $(CFLAGS) $?
heap.lo: $(srcdir)/heap/heap.c
	 $(CC) $(CFLAGS) $?
heap_auto.lo: $(srcdir)/heap/heap_auto.c
	 $(CC) $(CFLAGS) $?
heap_autop.lo: $(srcdir)/heap/heap_autop.c
	 $(CC) $(CFLAGS) $?
heap_backup.lo: $(srcdir)/heap/heap_backup.c
	 $(CC) $(CFLAGS) $?
heap_conv.lo: $(srcdir)/heap/heap_conv.c
	 $(CC) $(CFLAGS) $?
heap_method.lo: $(srcdir)/heap/heap_method.c
	 $(CC) $(CFLAGS) $?
heap_open.lo: $(srcdir)/heap/heap_open.c
	 $(CC) $(CFLAGS) $?
heap_rec.lo: $(srcdir)/heap/heap_rec.c
	 $(CC) $(CFLAGS) $?
heap_reclaim.lo: $(srcdir)/heap/heap_reclaim.c
	 $(CC) $(CFLAGS) $?
heap_stat.lo: $(srcdir)/heap/heap_stat.c
	 $(CC) $(CFLAGS) $?
heap_stub.lo: $(srcdir)/heap/heap_stub.c
	 $(CC) $(CFLAGS) $?
heap_upgrade.lo: $(srcdir)/heap/heap_upgrade.c
	 $(CC) $(CFLAGS) $?
heap_verify.lo: $(srcdir)/heap/heap_verify.c
	 $(CC) $(CFLAGS) $?
hmac.lo: $(srcdir)/hmac/hmac.c
	 $(CC) $(CFLAGS) $?
hsearch.lo: $(langdir)/hsearch/hsearch.c
	 $(CC) $(CFLAGS) $?
lock.lo: $(srcdir)/lock/lock.c $(srcdir)/lock/lock_alloc.incl
	 $(CC) $(CFLAGS) $(srcdir)/lock/lock.c
lock_deadlock.lo:$(srcdir)/lock/lock_deadlock.c
	 $(CC) $(CFLAGS) $?
lock_failchk.lo:$(srcdir)/lock/lock_failchk.c
	 $(CC) $(CFLAGS) $?
lock_id.lo:$(srcdir)/lock/lock_id.c
	 $(CC) $(CFLAGS) $?
lock_list.lo:$(srcdir)/lock/lock_list.c
	 $(CC) $(CFLAGS) $?
lock_method.lo:$(srcdir)/lock/lock_method.c
	 $(CC) $(CFLAGS) $?
lock_region.lo:$(srcdir)/lock/lock_region.c
	 $(CC) $(CFLAGS) $?
lock_stat.lo:$(srcdir)/lock/lock_stat.c
	 $(CC) $(CFLAGS) $?
lock_stub.lo: $(srcdir)/lock/lock_stub.c
	 $(CC) $(CFLAGS) $?
lock_timer.lo:$(srcdir)/lock/lock_timer.c
	 $(CC) $(CFLAGS) $?
lock_util.lo:$(srcdir)/lock/lock_util.c
	 $(CC) $(CFLAGS) $?
log.lo: $(srcdir)/log/log.c
	 $(CC) $(CFLAGS) $?
log_archive.lo: $(srcdir)/log/log_archive.c
	 $(CC) $(CFLAGS) $?
log_compare.lo: $(srcdir)/log/log_compare.c
	 $(CC) $(CFLAGS) $?
log_debug.lo: $(srcdir)/log/log_debug.c
	 $(CC) $(CFLAGS) $?
log_get.lo: $(srcdir)/log/log_get.c
	 $(CC) $(CFLAGS) $?
log_method.lo: $(srcdir)/log/log_method.c
	 $(CC) $(CFLAGS) $?
log_print.lo: $(srcdir)/log/log_print.c
	 $(CC) $(CFLAGS) $?
log_put.lo: $(srcdir)/log/log_put.c
	 $(CC) $(CFLAGS) $?
log_stat.lo: $(srcdir)/log/log_stat.c
	 $(CC) $(CFLAGS) $?
log_verify.lo: $(srcdir)/log/log_verify.c
	 $(CC) $(CFLAGS) $?
log_verify_auto.lo: $(srcdir)/log/log_verify_auto.c
	 $(CC) $(CFLAGS) $?
log_verify_int.lo: $(srcdir)/log/log_verify_int.c
	 $(CC) $(CFLAGS) $?
log_verify_util.lo: $(srcdir)/log/log_verify_util.c
	 $(CC) $(CFLAGS) $?
log_verify_stub.lo: $(srcdir)/log/log_verify_stub.c
	 $(CC) $(CFLAGS) $?
db_log_verify.lo: $(utildir)/db_log_verify.c
	 $(CC) $(CFLAGS) $?
mkpath.lo: $(srcdir)/common/mkpath.c
	 $(CC) $(CFLAGS) $?
mp_alloc.lo: $(srcdir)/mp/mp_alloc.c
	 $(CC) $(CFLAGS) $?
mp_bh.lo: $(srcdir)/mp/mp_bh.c
	 $(CC) $(CFLAGS) $?
mp_backup.lo: $(srcdir)/mp/mp_backup.c
	 $(CC) $(CFLAGS) $?
mp_fget.lo: $(srcdir)/mp/mp_fget.c
	 $(CC) $(CFLAGS) $?
mp_fmethod.lo: $(srcdir)/mp/mp_fmethod.c
	 $(CC) $(CFLAGS) $?
mp_fopen.lo: $(srcdir)/mp/mp_fopen.c
	 $(CC) $(CFLAGS) $?
mp_fput.lo: $(srcdir)/mp/mp_fput.c
	 $(CC) $(CFLAGS) $?
mp_fset.lo: $(srcdir)/mp/mp_fset.c
	 $(CC) $(CFLAGS) $?
mp_method.lo: $(srcdir)/mp/mp_method.c
	 $(CC) $(CFLAGS) $?
mp_mvcc.lo: $(srcdir)/mp/mp_mvcc.c
	 $(CC) $(CFLAGS) $?
mp_region.lo: $(srcdir)/mp/mp_region.c
	 $(CC) $(CFLAGS) $?
mp_register.lo: $(srcdir)/mp/mp_register.c
	 $(CC) $(CFLAGS) $?
mp_resize.lo: $(srcdir)/mp/mp_resize.c
	 $(CC) $(CFLAGS) $?
mp_stat.lo: $(srcdir)/mp/mp_stat.c
	 $(CC) $(CFLAGS) $?
mp_sync.lo: $(srcdir)/mp/mp_sync.c
	 $(CC) $(CFLAGS) $?
mp_trickle.lo: $(srcdir)/mp/mp_trickle.c
	 $(CC) $(CFLAGS) $?
mt19937db.lo: $(srcdir)/crypto/mersenne/mt19937db.c
	 $(CC) $(CFLAGS) $?
mut_alloc.lo: $(srcdir)/mutex/mut_alloc.c
	 $(CC) $(CFLAGS) $?
mut_failchk.lo: $(srcdir)/mutex/mut_failchk.c
	 $(CC) $(CFLAGS) $?
mut_method.lo: $(srcdir)/mutex/mut_method.c
	 $(CC) $(CFLAGS) $?
mut_pthread.lo: $(srcdir)/mutex/mut_pthread.c
	 $(CC) $(CFLAGS) $?
mut_region.lo: $(srcdir)/mutex/mut_region.c
	 $(CC) $(CFLAGS) $?
mut_stat.lo: $(srcdir)/mutex/mut_stat.c
	 $(CC) $(CFLAGS) $?
mut_stub.lo: $(srcdir)/mutex/mut_stub.c
	 $(CC) $(CFLAGS) $?
mut_tas.lo: $(srcdir)/mutex/mut_tas.c
	 $(CC) $(CFLAGS) $?
mut_win32.lo: $(srcdir)/mutex/mut_win32.c
	 $(CC) $(CFLAGS) $?
openflags.lo: $(srcdir)/common/openflags.c
	 $(CC) $(CFLAGS) $?
os_abs.lo: $(srcdir)/os/os_abs.c
	 $(CC) $(CFLAGS) $?
os_abort.lo: $(srcdir)/os/os_abort.c
	 $(CC) $(CFLAGS) $?
os_addrinfo.lo: $(srcdir)/os/os_addrinfo.c
	$(CC) $(CFLAGS) $?
os_alloc.lo: $(srcdir)/os/os_alloc.c
	 $(CC) $(CFLAGS) $?
os_clock.lo: $(srcdir)/os/os_clock.c
	 $(CC) $(CFLAGS) $?
os_config.lo: $(srcdir)/os/os_config.c
	 $(CC) $(CFLAGS) $?
os_cpu.lo: $(srcdir)/os/os_cpu.c
	 $(CC) $(CFLAGS) $?
os_ctime.lo: $(srcdir)/os/os_ctime.c
	 $(CC) $(CFLAGS) $?
os_dir.lo: $(srcdir)/os/os_dir.c
	 $(CC) $(CFLAGS) $?
os_errno.lo: $(srcdir)/os/os_errno.c
	 $(CC) $(CFLAGS) $?
os_fid.lo: $(srcdir)/os/os_fid.c
	 $(CC) $(CFLAGS) $?
os_flock.lo: $(srcdir)/os/os_flock.c
	 $(CC) $(CFLAGS) $?
os_fsync.lo: $(srcdir)/os/os_fsync.c
	 $(CC) $(CFLAGS) $?
os_getenv.lo: $(srcdir)/os/os_getenv.c
	 $(CC) $(CFLAGS) $?
os_handle.lo: $(srcdir)/os/os_handle.c
	 $(CC) $(CFLAGS) $?
os_map.lo: $(srcdir)/os/os_map.c
	 $(CC) $(CFLAGS) $?
os_method.lo: $(srcdir)/common/os_method.c
	 $(CC) $(CFLAGS) $?
os_mkdir.lo: $(srcdir)/os/os_mkdir.c
	 $(CC) $(CFLAGS) $?
os_open.lo: $(srcdir)/os/os_open.c
	 $(CC) $(CFLAGS) $?
os_path.lo: $(srcdir)/os/os_path.c
	 $(CC) $(CFLAGS) $?
os_pid.lo: $(srcdir)/os/os_pid.c
	 $(CC) $(CFLAGS) $?
os_qnx_fsync.lo: $(srcdir)/os_qnx/os_qnx_fsync.c
	 $(CC) $(CFLAGS) $?
os_qnx_open.lo: $(srcdir)/os_qnx/os_qnx_open.c
	 $(CC) $(CFLAGS) $?
os_rename.lo: $(srcdir)/os/os_rename.c
	 $(CC) $(CFLAGS) $?
os_rmdir.lo: $(srcdir)/os/os_rmdir.c
	 $(CC) $(CFLAGS) $?
os_root.lo: $(srcdir)/os/os_root.c
	 $(CC) $(CFLAGS) $?
os_rpath.lo: $(srcdir)/os/os_rpath.c
	 $(CC) $(CFLAGS) $?
os_rw.lo: $(srcdir)/os/os_rw.c
	 $(CC) $(CFLAGS) $?
os_seek.lo: $(srcdir)/os/os_seek.c
	 $(CC) $(CFLAGS) $?
os_stack.lo: $(srcdir)/os/os_stack.c
	 $(CC) $(CFLAGS) $?
os_stat.lo: $(srcdir)/os/os_stat.c
	 $(CC) $(CFLAGS) $?
os_tmpdir.lo: $(srcdir)/os/os_tmpdir.c
	 $(CC) $(CFLAGS) $?
os_truncate.lo: $(srcdir)/os/os_truncate.c
	 $(CC) $(CFLAGS) $?
os_uid.lo: $(srcdir)/os/os_uid.c
	 $(CC) $(CFLAGS) $?
os_unlink.lo: $(srcdir)/os/os_unlink.c
	 $(CC) $(CFLAGS) $?
os_yield.lo: $(srcdir)/os/os_yield.c
	 $(CC) $(CFLAGS) $?
partition.lo: $(srcdir)/db/partition.c
	 $(CC) $(CFLAGS) $?
qam.lo: $(srcdir)/qam/qam.c
	 $(CC) $(CFLAGS) $?
qam_auto.lo: $(srcdir)/qam/qam_auto.c
	 $(CC) $(CFLAGS) $?
qam_autop.lo: $(srcdir)/qam/qam_autop.c
	 $(CC) $(CFLAGS) $?
qam_conv.lo: $(srcdir)/qam/qam_conv.c
	 $(CC) $(CFLAGS) $?
qam_files.lo: $(srcdir)/qam/qam_files.c
	 $(CC) $(CFLAGS) $?
qam_method.lo: $(srcdir)/qam/qam_method.c
	 $(CC) $(CFLAGS) $?
qam_open.lo: $(srcdir)/qam/qam_open.c
	 $(CC) $(CFLAGS) $?
qam_rec.lo: $(srcdir)/qam/qam_rec.c
	 $(CC) $(CFLAGS) $?
qam_stat.lo: $(srcdir)/qam/qam_stat.c
	 $(CC) $(CFLAGS) $?
qam_stub.lo: $(srcdir)/qam/qam_stub.c
	 $(CC) $(CFLAGS) $?
qam_upgrade.lo: $(srcdir)/qam/qam_upgrade.c
	 $(CC) $(CFLAGS) $?
qam_verify.lo: $(srcdir)/qam/qam_verify.c
	 $(CC) $(CFLAGS) $?
rep_automsg.lo: $(srcdir)/rep/rep_automsg.c
	 $(CC) $(CFLAGS) $?
rep_backup.lo: $(srcdir)/rep/rep_backup.c
	 $(CC) $(CFLAGS) $?
rep_elect.lo: $(srcdir)/rep/rep_elect.c
	 $(CC) $(CFLAGS) $?
rep_lease.lo: $(srcdir)/rep/rep_lease.c
	 $(CC) $(CFLAGS) $?
rep_log.lo: $(srcdir)/rep/rep_log.c
	 $(CC) $(CFLAGS) $?
rep_method.lo: $(srcdir)/rep/rep_method.c
	 $(CC) $(CFLAGS) $?
rep_record.lo: $(srcdir)/rep/rep_record.c
	 $(CC) $(CFLAGS) $?
rep_region.lo: $(srcdir)/rep/rep_region.c
	 $(CC) $(CFLAGS) $?
rep_stub.lo: $(srcdir)/rep/rep_stub.c
	 $(CC) $(CFLAGS) $?
rep_stat.lo: $(srcdir)/rep/rep_stat.c
	 $(CC) $(CFLAGS) $?
rep_util.lo: $(srcdir)/rep/rep_util.c
	 $(CC) $(CFLAGS) $?
rep_verify.lo: $(srcdir)/rep/rep_verify.c
	 $(CC) $(CFLAGS) $?
repmgr_auto.lo: $(srcdir)/repmgr/repmgr_auto.c
	 $(CC) $(CFLAGS) $?
repmgr_automsg.lo: $(srcdir)/repmgr/repmgr_automsg.c
	 $(CC) $(CFLAGS) $?
repmgr_autop.lo: $(srcdir)/repmgr/repmgr_autop.c
	 $(CC) $(CFLAGS) $?
repmgr_elect.lo: $(srcdir)/repmgr/repmgr_elect.c
	$(CC) $(CFLAGS) $?
repmgr_method.lo: $(srcdir)/repmgr/repmgr_method.c
	$(CC) $(CFLAGS) $?
repmgr_msg.lo: $(srcdir)/repmgr/repmgr_msg.c
	$(CC) $(CFLAGS) $?
repmgr_net.lo: $(srcdir)/repmgr/repmgr_net.c
	$(CC) $(CFLAGS) $?
repmgr_posix.lo: $(srcdir)/repmgr/repmgr_posix.c
	$(CC) $(CFLAGS) $?
repmgr_queue.lo: $(srcdir)/repmgr/repmgr_queue.c
	$(CC) $(CFLAGS) $?
repmgr_rec.lo: $(srcdir)/repmgr/repmgr_rec.c
	$(CC) $(CFLAGS) $?
repmgr_sel.lo: $(srcdir)/repmgr/repmgr_sel.c
	$(CC) $(CFLAGS) $?
repmgr_stat.lo: $(srcdir)/repmgr/repmgr_stat.c
	$(CC) $(CFLAGS) $?
repmgr_stub.lo: $(srcdir)/repmgr/repmgr_stub.c
	$(CC) $(CFLAGS) $?
repmgr_util.lo: $(srcdir)/repmgr/repmgr_util.c
	$(CC) $(CFLAGS) $?
rijndael-alg-fst.lo: $(srcdir)/crypto/rijndael/rijndael-alg-fst.c
	$(CC) $(CFLAGS) $?
rijndael-api-fst.lo: $(srcdir)/crypto/rijndael/rijndael-api-fst.c
	$(CC) $(CFLAGS) $?
seq_stat.lo: $(srcdir)/sequence/seq_stat.c
	 $(CC) $(CFLAGS) $?
sequence.lo: $(srcdir)/sequence/sequence.c
	 $(CC) $(CFLAGS) $?
sha1.lo: $(srcdir)/hmac/sha1.c
	$(CC) $(CFLAGS) $?
stat_stub.lo: $(srcdir)/common/stat_stub.c
	 $(CC) $(CFLAGS) $?
txn.lo: $(srcdir)/txn/txn.c
	 $(CC) $(CFLAGS) $?
txn_auto.lo: $(srcdir)/txn/txn_auto.c
	 $(CC) $(CFLAGS) $?
txn_autop.lo: $(srcdir)/txn/txn_autop.c
	 $(CC) $(CFLAGS) $?
txn_chkpt.lo: $(srcdir)/txn/txn_chkpt.c
	 $(CC) $(CFLAGS) $?
txn_failchk.lo: $(srcdir)/txn/txn_failchk.c
	 $(CC) $(CFLAGS) $?
txn_method.lo: $(srcdir)/txn/txn_method.c
	 $(CC) $(CFLAGS) $?
txn_rec.lo: $(srcdir)/txn/txn_rec.c
	 $(CC) $(CFLAGS) $?
txn_recover.lo: $(srcdir)/txn/txn_recover.c
	 $(CC) $(CFLAGS) $?
txn_region.lo: $(srcdir)/txn/txn_region.c
	 $(CC) $(CFLAGS) $?
txn_stat.lo: $(srcdir)/txn/txn_stat.c
	 $(CC) $(CFLAGS) $?
txn_util.lo: $(srcdir)/txn/txn_util.c
	 $(CC) $(CFLAGS) $?
util_arg.lo: $(srcdir)/common/util_arg.c
	 $(CC) $(CFLAGS) $?
util_cache.lo: $(srcdir)/common/util_cache.c
	 $(CC) $(CFLAGS) $?
util_env.lo: $(srcdir)/common/util_env.c
	 $(CC) $(CFLAGS) $?
util_log.lo: $(srcdir)/common/util_log.c
	 $(CC) $(CFLAGS) $?
util_sig.lo: $(srcdir)/common/util_sig.c
	 $(CC) $(CFLAGS) $?
util_ver_check.lo: $(srcdir)/common/util_ver_check.c
	 $(CC) $(CFLAGS) $?
uts4_cc.lo: $(srcdir)/mutex/uts4_cc.s
	$(AS) $(ASFLAGS) -o $@ $?
xa.lo: $(srcdir)/xa/xa.c
	$(CC) $(CFLAGS) $?
xa_map.lo: $(srcdir)/xa/xa_map.c
	$(CC) $(CFLAGS) $?
zerofill.lo: $(srcdir)/common/zerofill.c
	 $(CC) $(CFLAGS) $?

##################################################
# C++ API build rules.
##################################################
cxx_channel.lo: $(langdir)/cxx/cxx_channel.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_db.lo: $(langdir)/cxx/cxx_db.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_dbc.lo: $(langdir)/cxx/cxx_dbc.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_dbstream.lo: $(langdir)/cxx/cxx_dbstream.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_dbt.lo: $(langdir)/cxx/cxx_dbt.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_env.lo: $(langdir)/cxx/cxx_env.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_except.lo: $(langdir)/cxx/cxx_except.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_lock.lo: $(langdir)/cxx/cxx_lock.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_logc.lo: $(langdir)/cxx/cxx_logc.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_mpool.lo: $(langdir)/cxx/cxx_mpool.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_multi.lo: $(langdir)/cxx/cxx_multi.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_rid.lo: $(langdir)/cxx/cxx_rid.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_seq.lo: $(langdir)/cxx/cxx_seq.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_site.lo: $(langdir)/cxx/cxx_site.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_txn.lo: $(langdir)/cxx/cxx_txn.cpp
	$(CXX) $(CXXFLAGS) $?

##################################################
# Java API build rules.
##################################################
db_java_wrap.lo: $(langdir)/java/libdb_java/db_java_wrap.c
	$(CC) $(CFLAGS) $(SWIGCFLAGS) $?

##################################################
# SQL API build rules.
##################################################
sqlite3.lo: $(langdir)/sql/generated/sqlite3.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?
shell.lo: $(langdir)/sql/sqlite/src/shell.c
	$(CC) $(CFLAGS) $(SQLFLAGS) $?

##################################################
# STL API build rules.
##################################################
dbstl_container.lo: $(langdir)/cxx/stl/dbstl_container.cpp
	$(CXX) $(STLFLAGS) $?
dbstl_resource_manager.lo: $(langdir)/cxx/stl/dbstl_resource_manager.cpp
	$(CXX) $(STLFLAGS) $?

##################################################
# Tcl API build rules.
##################################################
tcl_compat.lo: $(TCL_SRCDIR)/tcl_compat.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_db.lo: $(TCL_SRCDIR)/tcl_db.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_db_pkg.lo: $(TCL_SRCDIR)/tcl_db_pkg.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_dbcursor.lo: $(TCL_SRCDIR)/tcl_dbcursor.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_dbstream.lo: $(TCL_SRCDIR)/tcl_dbstream.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_env.lo: $(TCL_SRCDIR)/tcl_env.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_internal.lo: $(TCL_SRCDIR)/tcl_internal.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_lock.lo: $(TCL_SRCDIR)/tcl_lock.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_log.lo: $(TCL_SRCDIR)/tcl_log.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_mp.lo: $(TCL_SRCDIR)/tcl_mp.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_mutex.lo: $(TCL_SRCDIR)/tcl_mutex.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_rep.lo: $(TCL_SRCDIR)/tcl_rep.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_seq.lo: $(TCL_SRCDIR)/tcl_seq.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_txn.lo: $(TCL_SRCDIR)/tcl_txn.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_util.lo: $(TCL_SRCDIR)/tcl_util.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?

##################################################
# Utility build rules.
##################################################
db_archive.lo: $(utildir)/db_archive.c
	$(CC) $(CFLAGS) $?
db_checkpoint.lo: $(utildir)/db_checkpoint.c
	$(CC) $(CFLAGS) $?
db_convert.lo: $(utildir)/db_convert.c
	$(CC) $(CFLAGS) $?
db_deadlock.lo: $(utildir)/db_deadlock.c
	$(CC) $(CFLAGS) $?
db_dump.lo: $(utildir)/db_dump.c
	$(CC) $(CFLAGS) $?
db_dump185.lo: $(utildir)/db_dump185.c
	$(CC) $(DB185INC) $?
db_hotbackup.lo: $(utildir)/db_hotbackup.c
	$(CC) $(CFLAGS) $?
db_load.lo: $(utildir)/db_load.c
	$(CC) $(CFLAGS) $?
db_printlog.lo: $(utildir)/db_printlog.c
	$(CC) $(CFLAGS) $?
db_recover.lo: $(utildir)/db_recover.c
	$(CC) $(CFLAGS) $?
db_replicate.lo: $(utildir)/db_replicate.c
	$(CC) $(CFLAGS) $?
db_stat.lo: $(utildir)/db_stat.c
	$(CC) $(CFLAGS) $?
db_tuner.lo: $(utildir)/db_tuner.c
	$(CC) $(CFLAGS) $?
db_upgrade.lo: $(utildir)/db_upgrade.c
	$(CC) $(CFLAGS) $?
db_verify.lo: $(utildir)/db_verify.c
	$(CC) $(CFLAGS) $?

##################################################
# C library replacement files.
##################################################
atoi.lo: $(srcdir)/clib/atoi.c
	$(CC) $(CFLAGS) $?
atol.lo: $(srcdir)/clib/atol.c
	$(CC) $(CFLAGS) $?
bsearch.lo: $(srcdir)/clib/bsearch.c
	$(CC) $(CFLAGS) $?
getcwd.lo: $(srcdir)/clib/getcwd.c
	$(CC) $(CFLAGS) $?
getopt.lo: $(srcdir)/clib/getopt.c
	$(CC) $(CFLAGS) $?
isalpha.lo: $(srcdir)/clib/isalpha.c
	$(CC) $(CFLAGS) $?
isdigit.lo: $(srcdir)/clib/isdigit.c
	$(CC) $(CFLAGS) $?
isprint.lo: $(srcdir)/clib/isprint.c
	$(CC) $(CFLAGS) $?
isspace.lo: $(srcdir)/clib/isspace.c
	$(CC) $(CFLAGS) $?
memcmp.lo: $(srcdir)/clib/memcmp.c
	$(CC) $(CFLAGS) $?
memcpy.lo: $(srcdir)/clib/memmove.c
	$(CC) -DMEMCOPY $(CFLAGS) $? -o $@
memmove.lo: $(srcdir)/clib/memmove.c
	$(CC) -DMEMMOVE $(CFLAGS) $?
printf.lo: $(srcdir)/clib/printf.c
	$(CC) $(CFLAGS) $?
qsort.lo: $(srcdir)/clib/qsort.c
	$(CC) $(CFLAGS) $?
raise.lo: $(srcdir)/clib/raise.c
	$(CC) $(CFLAGS) $?
rand.lo: $(srcdir)/clib/rand.c
	$(CC) $(CFLAGS) $?
strcasecmp.lo: $(srcdir)/clib/strcasecmp.c
	$(CC) $(CFLAGS) $?
strdup.lo: $(srcdir)/clib/strdup.c
	$(CC) $(CFLAGS) $?
snprintf.lo: $(srcdir)/clib/snprintf.c
	$(CC) $(CFLAGS) $?
strcat.lo: $(srcdir)/clib/strcat.c
	$(CC) $(CFLAGS) $?
strchr.lo: $(srcdir)/clib/strchr.c
	$(CC) $(CFLAGS) $?
strerror.lo: $(srcdir)/clib/strerror.c
	$(CC) $(CFLAGS) $?
strncat.lo: $(srcdir)/clib/strncat.c
	$(CC) $(CFLAGS) $?
strncmp.lo: $(srcdir)/clib/strncmp.c
	$(CC) $(CFLAGS) $?
strrchr.lo: $(srcdir)/clib/strrchr.c
	$(CC) $(CFLAGS) $?
strsep.lo: $(srcdir)/clib/strsep.c
	$(CC) $(CFLAGS) $?
strtol.lo: $(srcdir)/clib/strtol.c
	$(CC) $(CFLAGS) $?
strtoul.lo: $(srcdir)/clib/strtoul.c
	$(CC) $(CFLAGS) $?
time.lo: $(srcdir)/clib/time.c
	$(CC) $(CFLAGS) $?

##################################################
# Performance Event Monitoring build rules
##################################################

# DTrace rules:
# The DTrace DB provider description (db_provider.d) is included in the
# distribution. It needs to be rebuilt when adding events to
# $(distdir)/events.in.  The C/C++ header file db_provider.h is
# created at configure time, and is rebuilt when db_provider.d changes.
# It has lower-case versions of the event class and type names.
#
# To list the DB static probes, try:
#	[pfexec | sudo] dtrace -l -n 'bdb$target:::'  -c "<application> <args>"
#	[sudo] stap -l 'process(".libs/libdb-18.1.so").mark("*")'
listevents listprobes: 
	

$(DTRACE_PROVIDER): $(distdir)/gen_provider.pl $(distdir)/events.in
	-@$(RM) $@
	$(PERL) $(distdir)/gen_provider.pl $(distdir)/events.in > $@

SED_PROVIDER_PATTERN='/^\#define[ 	]*BDB_[A-Z_]*(/y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/'
db_provider.h : $(DTRACE_PROVIDER)
	-@$(RM) $@
	@if test -z "$(DTRACE)" ; then echo "$@ is used only with dtrace or stap"; exit 1; fi
	($(DTRACE) -h -C -I$(utildir)/dtrace -s $(DTRACE_PROVIDER) -o $@ && \
	    $(MV) $@ $@.tmp && \
	    $(SED) -e $(SED_PROVIDER_PATTERN) $@.tmp > $@) || $(RM) $@
	@$(RM) $@.tmp

# DTrace on platforms which require dtrace -G (e.g., Solaris) need to include
# an extra object file when linking. This placeholder 'db_provider.lo' has a
# special update rule which knows that the PIC object files may be in .libs.
# If that directory exists, the object files located there are updated by a
# separate dtrace -G call.
#
#	Should this be added to libtool's --mode=link step on Solaris?
#
DTRACE_OFILES=`echo $(DTRACE_OBJS) " " | $(SED) -e 's/\.lo /\.o /g'`

db_provider.lo: db_provider.c $(DTRACE_OBJS) $(DTRACE_PROVIDER)
	$(RM) db_provider.o .libs/db_provider.o
	@# A compilation warning such as 'empty translation unit' is harmless.
	$(CC) $(CFLAGS) db_provider.c
	if test -f db_provider.o ; then \
		$(DTRACE) -G -C -I$(utildir)/dtrace -s $(DTRACE_PROVIDER) $(DTRACE_OFILES) ; \
	fi
	if test -f .libs/db_provider.o ; then \
	    (cd .libs && \
	      $(DTRACE) -G -C -I../$(utildir)/dtrace -s ../$(DTRACE_PROVIDER) $(DTRACE_OFILES)) || \
	      $(RM) $@ ; \
	fi

# db_provider.c is created as an empty file at configure time with
# --enable-dtrace. If missing then configure should be re-run.
# So far only Solaris needs it.
db_provider.c:
	@echo "The file db_provider.lo is used only for configurations requiring dtrace -G"
	@echo "Re-run configure with --enable-dtrace if needed"
	@exit 1
