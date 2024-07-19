/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.dbpage;

import com.sleepycat.db.CompactConfig;
import com.sleepycat.db.CompactStats;
import com.sleepycat.db.DatabaseConfig;
import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.DatabaseType;
import com.sleepycat.db.DeadlockException;

import java.util.logging.Level;
import java.util.logging.Logger;

import javafx.concurrent.Task;
import db_gui.BDBInitializable;
import db_gui.BDBState;
import db_gui.EnableRunnable;
import javafx.application.Platform;

/**
 * CompactTask compacts a database in a background thread.
 */
public class CompactTask extends Task<Void> {
    final private String file;

    /**
     * Default constructor.
     *
     * @param dbFile - The database file to be compacted.
     */
    public CompactTask(String dbFile) {
        file = dbFile;
    }

    /**
     * Opens the database file, configures compaction to return any free pages
     * to the file system, runs compaction, prints out the number of pages
     * returned to the database, and closes the database.
     *
     * @return
     * @throws Exception
     */
    @Override
    protected Void call() throws Exception {
        BDBState bdbState = BDBState.getBDBState();
        DatabaseType type = DatabaseType.UNKNOWN;
        DatabaseConfig config = new DatabaseConfig();
        config.setType(type);
        bdbState.printFeedback("Opening the database.");
        if (bdbState.openDatabase(file, null, config)) {
            try {
                type = bdbState.getDb().getConfig().getType();
                if (type == DatabaseType.HEAP || type == DatabaseType.QUEUE) {
                    bdbState.printFeedback(
                            "Error, cannot compact Heap and Queue databases.");
                    bdbState.closeDatabase();
                    Platform.runLater(new EnableRunnable());
                    return null;
                }
                CompactConfig cconfig = new CompactConfig();
                cconfig.setFreeSpace(true);
                bdbState.printFeedback("Compacting the database.");
                CompactStats compact = bdbState.getDb().compact(
                        bdbState.beginTxn(), null, null, null, cconfig);
                bdbState.commitTransaction();
                bdbState.printFeedback("Success compacting database: "
                        + file + ", " + compact.getPagesTruncated()
                        + " pages returned to the filesystem.");
            } catch (DeadlockException dex) {
                BDBInitializable.handleDeadlockException();
            } catch (DatabaseException ex) {
                Logger.getLogger(
                        DatabaseOpenController.class.getName()).log(
                        Level.SEVERE, null, ex);
                bdbState.printFeedback(
                        "Error compacting database: " + ex.getMessage());
                bdbState.abortTransaction();
            } finally {
                bdbState.closeDatabase();
            }
        }
        Platform.runLater(new EnableRunnable());
        return null;
    }
}
