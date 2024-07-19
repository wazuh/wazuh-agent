/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import com.sleepycat.db.CheckpointConfig;
import com.sleepycat.db.DatabaseException;
import javafx.concurrent.Task;
import db_gui.BDBState;
import db_gui.EnableRunnable;
import javafx.application.Platform;

/**
 * RemoveLogTask performs the removal of old logs in a background thread.
 */
public class RemoveLogTask extends Task<Void> {
    private final EnvConfig config;

    /**
     * Constructor.
     *
     * @param econfig - Environment configurations.
     */
    public RemoveLogTask(EnvConfig econfig) {
        config = econfig;
    }

    /**
     * Opens the environment, runs a checkpoint, removes the old logs, then
     * closes the environment.
     *
     * @return - null.
     * @throws Exception
     */
    @Override
    protected Void call() throws Exception {
        BDBState bdbState = BDBState.getBDBState();
        if (bdbState.openEnvironment(config, true, false)) {
            bdbState.printFeedback("Environment successfully opened.");
            if (!bdbState.isTransactional()) {
                bdbState.printFeedback(
                        "Error, remove logs requires that the environment is transactional.");
                bdbState.closeEnvironment();
                Platform.runLater(new EnableRunnable());
                return null;
            }
            try {
                bdbState.printFeedback("Executing a checkpoint.");
                bdbState.getEnv().checkpoint(CheckpointConfig.DEFAULT);
                bdbState.printFeedback("Removing logs.");
                bdbState.getEnv().removeOldLogFiles();
                bdbState.printFeedback("Old logs successfully removed.");
            } catch (DatabaseException ex) {
                bdbState.printFeedback(
                        "Error removing logs: " + ex.getMessage() + ".");
            }
            bdbState.closeEnvironment();
        }
        Platform.runLater(new EnableRunnable());
        return null;
    }
}

