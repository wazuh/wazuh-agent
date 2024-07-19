/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_guitest;

import com.sleepycat.db.DatabaseConfig;
import com.sleepycat.db.DatabaseType;
import db_gui.envpage.EnvConfig;

import java.io.File;
import java.util.concurrent.Callable;

import db_gui.BDBInitializable;
import db_gui.BDBState;

/**
 * DisplayAccessPageTask is used to display the data access page, which
 * requires that the database type and transactional status of the environment
 * be known when the page is created.
 */
public class DisplayAccessPageTask implements Callable<Void> {
    private final boolean transactional;
    private final DatabaseType type;

    public DisplayAccessPageTask(DatabaseType type, boolean transactional) {
        this.type = type;
        this.transactional = transactional;
    }


    @Override
    public Void call() throws Exception {
        BDBState state = BDBState.getBDBState();
        if (state.getEnv() == null) {
            EnvConfig envConfig = new EnvConfig();
            envConfig.setHome(new File(BDBGUITestConfig.ENV_DIR));
            state.openEnvironment(envConfig, transactional, true);
        }
        if (state.getDb() == null) {
            String subdb = null;
            DatabaseConfig config = new DatabaseConfig();
            config.setType(type);
            config.setAllowCreate(true);
            if (type != DatabaseType.HEAP && type != DatabaseType.QUEUE)
                subdb = BDBGUITestConfig.DATABASE_NAME2;
            if (type == DatabaseType.QUEUE) {
                config.setRecordLength(64);
                config.setRecordPad(0);
            }
            state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, subdb, config);
        }
        BDBInitializable.forwardDataAccessPage(state, type);
        return null;
    }
}
