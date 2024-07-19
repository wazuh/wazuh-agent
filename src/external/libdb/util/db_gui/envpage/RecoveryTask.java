/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import javafx.concurrent.Task;
import db_gui.BDBState;
import db_gui.EnableRunnable;
import javafx.application.Platform;

/**
 * RecoveryTask performs environment recovery in a background thread.
 */
public class RecoveryTask extends Task<Void> {
    private final EnvConfig config;
    private final boolean catastrophic;

    /**
     * Constructor.
     *
     * @param econfig - Environment configurations.
     * @param isCatastrophic - Whether normal or catastrophic recovery should
     * be
     * performed.
     */
    public RecoveryTask(EnvConfig econfig, boolean isCatastrophic) {
        config = econfig;
        catastrophic = isCatastrophic;
    }

    /**
     * Opens the environment with recovery, then closes the environment.
     *
     * @return
     * @throws Exception
     */
    @Override
    protected Void call() throws Exception {
        BDBState bdbState = BDBState.getBDBState();
        bdbState.printFeedback("Performing recovery on the environment.");
        if (bdbState.recoverEnvironment(config, catastrophic)) {
            bdbState.printFeedback("Environment recovery successful.");
            bdbState.closeEnvironment();
        }
        Platform.runLater(new EnableRunnable());
        return null;
    }

}
