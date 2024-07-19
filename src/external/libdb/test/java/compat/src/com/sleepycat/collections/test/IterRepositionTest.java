/*-
 * Copyright (C) 2002, 2018, Oracle and/or its affiliates. All rights reserved.
 *
 * This file was distributed by Oracle as part of a version of Oracle Berkeley
 * DB Java Edition made available at:
 *
 * http://www.oracle.com/technetwork/database/database-technologies/berkeleydb/downloads/index.html
 *
 * Please see the LICENSE file included in the top-level directory of the
 * appropriate version of Oracle Berkeley DB Java Edition for a copy of the
 * license and additional information.
 */
package com.sleepycat.collections.test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;

import java.util.Iterator;

import com.sleepycat.bind.EntryBinding;
import com.sleepycat.bind.tuple.IntegerBinding;
import com.sleepycat.collections.StoredSortedMap;
import com.sleepycat.collections.StoredValueSet;
import com.sleepycat.compat.DbCompat;
import com.sleepycat.db.Database;
import com.sleepycat.db.DatabaseConfig;
import com.sleepycat.db.Environment;
import com.sleepycat.util.test.TestBase;
import com.sleepycat.util.test.TestEnv;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

/**
 * In general the BlockIterator (default iterator() of the collections API)
 * is tested by CollectionTest. Additional testing for repositioning is here.
 * The BlockIterator does not keep an open cursor or hold locks, it caches a
 * block of records at a time. When reaching the end of the block it must
 * reposition to the next record, which is a little tricky in JE and deserves
 * special testing.
 */
public class IterRepositionTest extends TestBase {

    private Environment env;

    @Before
    public void setUp() throws Exception {
        super.setUp();
        env = TestEnv.TXN.open("IterRepositionTest");
    }

    @After
    public void tearDown() throws Exception {
        env.close();
        super.tearDown();
    }

    /**
     * Tests a bug when the BlockIterator repositioned at an earlier key when
     * the last key in the previous block was deleted.
     */
    @Test
    public void testDeleteLastKeyBug() throws Exception {

        final Database db = openDb("foo");

        final EntryBinding<Integer> binding = new IntegerBinding();

        final StoredSortedMap<Integer, Integer> map  = new StoredSortedMap<>(
            db, binding, binding, true);

        final StoredValueSet<Integer> set =
            (StoredValueSet<Integer>) map.values();

        /*
         * Use a block size of 5 and fill one block.
         */
        set.setIteratorBlockSize(5);

        for (int i = 1; i <= 5; i += 1) {
            assertNull(map.put(0, i));
        }

        /*
         * Move iterator to last (5th) record.
         */
        final Iterator<Integer> iter = set.iterator();
        for (Integer i = 1; i <= 5; i += 1) {
            assertEquals(i, iter.next());
        }

        /*
         * Delete the last (5th) record using a different iterator.
         */
        final Iterator<Integer> deleteIter = set.iterator();
        for (Integer i = 1; i <= 5; i += 1) {
            assertEquals(i, deleteIter.next());
        }
        deleteIter.remove();

        /*
         * Prior to the bug fix, hasNext below returned true because the
         * DataCursor.repositionRange method positioned to an earlier record.
         */
        assertFalse(iter.hasNext());

        db.close();
    }

    private Database openDb(final String file)
        throws Exception {

        final DatabaseConfig config = new DatabaseConfig();
        DbCompat.setTypeBtree(config);
        config.setSortedDuplicates(true);
        config.setAllowCreate(true);

        return DbCompat.testOpenDatabase(env, null, file, null, config);
    }
}
