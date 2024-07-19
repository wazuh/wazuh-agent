/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import com.sleepycat.db.BackupOptions;
import com.sleepycat.db.CheckpointConfig;
import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.EnvironmentConfig;

import java.io.File;
import java.util.logging.Level;
import java.util.logging.Logger;

import javafx.concurrent.Task;
import db_gui.BDBState;
import db_gui.EnableRunnable;
import javafx.application.Platform;

/**
 * BackupTask performs hot backup in a background thread.
 */
public class BackupTask extends Task<Void> {
    private final EnvConfig config;
    private final File backupHome;
    private final BackupOptions options;

    /**
     * Constructor.
     *
     * @param econfig - Environment configurations.
     * @param backupHome - Home directory of the backup.
     * @param options - Backup configurations.
     */
    public BackupTask(EnvConfig econfig,
            File backupHome, BackupOptions options) {
        config = econfig;
        this.backupHome = backupHome;
        this.options = options;
    }

    /**
     * Performs hot backup by opening the environment, setting the hot backup
     * in progress flag (which runs a checkpoint), performs backup, runs
     * catastrophic recovery on the backup, and closes the environment.
     *
     * @return - null.
     * @throws Exception
     */
    @Override
    protected Void call() throws Exception {
        BDBState bdbState = BDBState.getBDBState();
        EnableRunnable enable = new EnableRunnable();
        EnvironmentConfig envConfig = new EnvironmentConfig();
        envConfig.setAllowCreate(true);
        if (bdbState.openEnvironment(config, envConfig)) {
            if (!bdbState.isTransactional()) {
                bdbState.printFeedback(
                        "Error, backup requires that the environment is transactional.");
                bdbState.closeEnvironment();
                Platform.runLater(enable);
                return null;
            }
            try {
                // Perform a checkpoint.
                bdbState.printFeedback("Performing a checkpoint.");
                CheckpointConfig cconfig = new CheckpointConfig();
                cconfig.setForce(true);
                bdbState.getEnv().checkpoint(cconfig);
                bdbState.printFeedback("Performing backup.");
                bdbState.getEnv().backup(backupHome.getAbsolutePath(), options);
                bdbState.closeEnvironment();
                if (!options.getUpdate()) {
                    bdbState.printFeedback(
                            "Running catastrophic recovery on the backup.");
                    bdbState.recoverEnvironment(config, true);
                    bdbState.closeEnvironment();
                }
                bdbState.printFeedback("Environment backup successful.");
            } catch (DatabaseException ex) {
                bdbState.printFeedback(
                        "Error performing environment backup: "
                                + ex.getMessage() + ".");
                Logger.getLogger(
                        EnvironmentBackupController.class.getName()).log(
                        Level.SEVERE, null, ex);
            } finally {
                bdbState.closeEnvironment();
            }
        } else {
            bdbState.printFeedback("Error opening environment.");
        }
        Platform.runLater(enable);
        return null;
    }

}
