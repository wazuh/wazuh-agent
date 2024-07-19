/*-
 * Copyright (c) 2002, 2020 Oracle and/or its affiliates.  All rights reserved.
 * 
 * See the file LICENSE for license information.
 *
 */


package com.sleepycat.db.test;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import com.sleepycat.db.*;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

import com.sleepycat.db.test.TestUtils;
public class LockTimeoutTest {
    public static final String LOCKTIMEOUTTEST_DBNAME = "locktimeouttest.db";
    @BeforeClass public static void ClassInit() {
        TestUtils.loadConfig(null);
        TestUtils.check_file_removed(TestUtils.getDBFileName(LOCKTIMEOUTTEST_DBNAME), true, true);
        TestUtils.removeall(true, true, TestUtils.BASETEST_DBDIR, TestUtils.getDBFileName(LOCKTIMEOUTTEST_DBNAME));
    }

    @AfterClass public static void ClassShutdown() {
        TestUtils.check_file_removed(TestUtils.getDBFileName(LOCKTIMEOUTTEST_DBNAME), true, true);
        TestUtils.removeall(true, true, TestUtils.BASETEST_DBDIR, TestUtils.getDBFileName(LOCKTIMEOUTTEST_DBNAME));
    }

    @Before public void PerTestInit()
        throws Exception {
    }

    @After public void PerTestShutdown()
        throws Exception {
    }
    /*
     * Test case implementations.
     * To disable a test mark it with @Ignore
     * To set a timeout(ms) notate like: @Test(timeout=1000)
     * To indicate an expected exception notate like: (expected=Exception)
     */

    @Test public void test1()
        throws DatabaseException, FileNotFoundException
    {
        EnvironmentConfig envConfig = new EnvironmentConfig();
        envConfig.setErrorPrefix("LockTimeout");
        envConfig.setErrorStream(System.err);
        envConfig.setAllowCreate(true);
        envConfig.setInitializeLocking(true);
        envConfig.setLockTimeout(10000);
        
        Environment dbenv = 
            new Environment(TestUtils.BASETEST_DBFILE, envConfig);
        int locker = dbenv.createLockerID();

        DatabaseEntry entry = new DatabaseEntry("LockObject".getBytes());
        LockRequest req = new LockRequest(
            LockOperation.GET_TIMEOUT, LockRequestMode.WRITE, entry);
        req.setTimeout(20000);
        LockRequest[] reqs = { req };
        dbenv.lockVector(locker, true, reqs);

        assertEquals(req.getTimeout(), 20000);
        dbenv.close();
    }
}
