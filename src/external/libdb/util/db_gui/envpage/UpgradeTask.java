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
import com.sleepycat.db.Environment;
import com.sleepycat.db.EnvironmentConfig;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.logging.Level;
import java.util.logging.Logger;

import javafx.concurrent.Task;
import db_gui.BDBState;
import db_gui.EnableRunnable;
import javafx.application.Platform;

/**
 * UpgradeTask performs database upgrades in a background thread.
 */
public class UpgradeTask extends Task<Void> {
    private final EnvConfig config;
    private final File[] databaseFiles;

    /**
     * Constructor.
     *
     * @param econfig - Contains environment configurations.
     * @param dbFiles - Database files to upgrade.
     */
    public UpgradeTask(EnvConfig econfig, File[] dbFiles) {
        config = econfig;
        databaseFiles = dbFiles;
    }

    /**
     * Removes the old environment, opens a new private environment, upgrades
     * each of the given databases, and closes the environment.
     *
     * @return - null.
     * @throws Exception
     */
    @Override
    protected Void call() throws Exception {
        BDBState bdbState = BDBState.getBDBState();
        try {
            Environment.remove(
                    config.getHome(), true, EnvironmentConfig.DEFAULT);
        } catch (DatabaseException | FileNotFoundException ex) {
            Logger.getLogger(
                    EnvironmentUpgradeController.class.getName()).log(
                    Level.SEVERE, null, ex);
            bdbState.printFeedback(
                    "Error removing old environment: " + ex.getMessage());
            Platform.runLater(new EnableRunnable());
            return null;
        }
        EnvironmentConfig envConfig = new EnvironmentConfig();
        envConfig.setPrivate(true);
        envConfig.setAllowCreate(true);
        if (bdbState.openEnvironment(config, envConfig)) {
            bdbState.printFeedback("Environment successfully opened.");
            for (File dbFile : databaseFiles) {
                try {
                    bdbState.printFeedback(
                            "Upgrading database: " + dbFile.getName() + ".");
                    DatabaseConfig dbConfig = new DatabaseConfig();
                    dbConfig.setErrorHandler(bdbState);
                    dbConfig.setMessageHandler(bdbState);
                    dbConfig.setFeedbackHandler(bdbState);
                    Database.upgrade(dbFile.getPath(), dbConfig);
                } catch (DatabaseException
                        | IllegalArgumentException | FileNotFoundException ex) {
                    Logger.getLogger(
                            EnvironmentUpgradeController.class.getName()).log(
                            Level.SEVERE, null, ex);
                    bdbState.printFeedback("Error upgrading database: "
                            + dbFile.getName() + ".  " + ex.getMessage());
                }
            }
            bdbState.printFeedback("Upgrade process finished.");
            bdbState.closeEnvironment();
        }
        Platform.runLater(new EnableRunnable());
        return null;
    }

}
