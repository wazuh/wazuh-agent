/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import com.sleepycat.db.LogVerifyConfig;
import javafx.concurrent.Task;
import db_gui.BDBState;
import db_gui.EnableRunnable;
import javafx.application.Platform;

/**
 * VerifyLogTask verifies the logs of the current environment as a background
 * thread.
 */
public class VerifyLogTask extends Task<Void> {
    private final EnvConfig config;
    private final LogVerifyConfig logConfig;

    /**
     * Constructor.
     *
     * @param econfig - Configurations for the environment to verify.
     * @param lconfig - Log specific configurations for the environment.
     */
    public VerifyLogTask(EnvConfig econfig, LogVerifyConfig lconfig) {
        config = econfig;
        logConfig = lconfig;
    }

    /**
     * Opens the environment, verifies the logs, then closes the environment.
     *
     * @return - null
     * @throws Exception
     */
    @Override
    protected Void call() throws Exception {
        BDBState bdbState = BDBState.getBDBState();
        if (bdbState.openEnvironment(config, true, false)) {
            bdbState.printFeedback("Environment successfully opened.");
            if (!bdbState.isTransactional()) {
                bdbState.printFeedback(
                        "Error, verify logs requires that the environment is transactional.");
                bdbState.closeEnvironment();
                Platform.runLater(new EnableRunnable());
                return null;
            }
            bdbState.printFeedback("Begin verifying logs.");
            if (bdbState.getEnv().logVerify(logConfig) != 0)
                bdbState.printFeedback("Log verification failed.");
            else
                bdbState.printFeedback("Log verification succeeded.");
            bdbState.closeEnvironment();
        }
        Platform.runLater(new EnableRunnable());
        return null;
    }
}
