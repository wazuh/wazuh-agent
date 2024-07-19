/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import com.sleepycat.db.Database;
import com.sleepycat.db.DatabaseConfig;
import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.VerifyConfig;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.logging.Level;
import java.util.logging.Logger;

import javafx.concurrent.Task;
import db_gui.BDBState;
import db_gui.EnableRunnable;
import javafx.application.Platform;

/**
 * VerifyDataTask verifies the given database files in a background thread.
 */
public class VerifyDataTask extends Task<Void> {
    private final EnvConfig config;
    private final File[] databaseFiles;

    /**
     * Constructor.
     *
     * @param econfig - Configurations for the environment.
     * @param dbFiles - Paths to the database files to be verified.
     */
    public VerifyDataTask(EnvConfig econfig, File[] dbFiles) {
        config = econfig;
        databaseFiles = dbFiles;
    }

    /**
     * Opens the environment, verifies each given database file, then closes
     * the environment.
     *
     * @return - null.
     * @throws Exception
     */
    @Override
    protected Void call() throws Exception {
        BDBState state = BDBState.getBDBState();
        DatabaseConfig dbConfig = new DatabaseConfig();
        dbConfig.setErrorHandler(state);
        dbConfig.setMessageHandler(state);
        dbConfig.setFeedbackHandler(state);
        if (state.openEnvironment(config, false, false)) {
            for (File dbFile : databaseFiles) {
                try {
                    if (Database.verify(dbFile.getPath(),
                            null, null, VerifyConfig.DEFAULT, dbConfig))
                        state.printFeedback("Success validating database: "
                                + dbFile.getName());
                    else
                        state.printFeedback("Failed validating database: "
                                + dbFile.getName());
                } catch (DatabaseException
                        | IllegalArgumentException | FileNotFoundException ex) {
                    Logger.getLogger(
                            EnvironmentUpgradeController.class.getName()).log(
                            Level.SEVERE, null, ex);
                    state.printFeedback(
                            "Error performing verification on database: "
                                    + dbFile.getName() + ".  The error is:  "
                                    + ex.getMessage());
                }
            }
            state.closeEnvironment();
        }
        Platform.runLater(new EnableRunnable());
        return null;
    }

}
