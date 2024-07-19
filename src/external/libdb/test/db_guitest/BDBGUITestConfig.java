/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_guitest;

import java.io.File;
import java.io.IOException;

/**
 * @author lfoutz
 */
public class BDBGUITestConfig {
    public final static String ENV_DIR = "." + File.separator + "TESTDIR";
    public final static String BACKUP_DIR = "." + File.separator + "BACKUPDIR";
    public final static String REGION_FILE =
            ENV_DIR + File.separator + "__db.001";
    public final static String DATABASE_NAME1 = "test1.db";
    public final static String DATABASE_NAME2 = "test2.db";
    public final static String LOG_DIR = "LOGS";
    public final static String EXT_FILE_DIR = "EXT";
    public final static String DATA1_DIR = "DATA1";
    public final static String DATA2_DIR = "DATA2";
    public final static String ENCRYPTION_PASSWORD = "abcde";
    public final static String DB_ENCRYPTION_PASSWORD = "edcba";
    public final static String CACHE_SIZE_STRING = "1048576";
    public final static long CACHE_SIZE = 1048576;
    public final static String LOG_FILE = "log.0000000001";

    /*
     * On Windows, it is possible for file to remain in a pending
     * delete state for a while after being deleted.  To deal with
     * this, after deleting the files the code gets a list of the
     * files in the folder and checks if they are still there,
     * if so pauses then checks again.  Note that checking
     * if the file still exists will return that it does not
     * when it is in the pending delete state.
     */
    private static void waitOnRemove(File dir) {
        for (int i = 0; i < 5; i++) {
            File[] files = dir.listFiles();
            boolean deleted = true;
            for (File file : files) {
                if (!file.isDirectory()) {
                    deleted = false;
                    break;
                }
            }
            if (deleted)
                return;
            else {
        /* Pause and check again. */
                try {
                    Thread.sleep(100);
                } catch (InterruptedException ex) {
                    Thread.currentThread().interrupt();
                }
            }
        }
    }

    static private void removeFiles(File dir) {
        File[] fileNames = dir.listFiles();
        for (File file : fileNames) {
            if (file.isDirectory())
                removeFiles(file);
            file.delete();
        }
        waitOnRemove(dir);
    }

    /*
     * Clear the Environment.
     */
    public static void fileRemove(String envPath) {
        File dir = new File(envPath);
        if (dir.isDirectory()) {
            removeFiles(dir);
        }
    }

    public static void cleanEnvironment() {
        File dir = new File(ENV_DIR);
        if (dir.exists()) {
            removeFiles(dir);
        }
    }

    /**
     * Create the home, log, external file, and data directories.
     *
     * @throws IOException
     */
    public static void createEnvironmentDirectories() throws IOException {
        File dir = new File(ENV_DIR);
        if (!dir.exists()) {
            dir.mkdirs();
        }
        dir = new File(BDBGUITestConfig.ENV_DIR + File.separator + LOG_DIR);
        if (!dir.exists()) {
            dir.mkdirs();
        }
        dir = new File(BDBGUITestConfig.ENV_DIR
                + File.separator + EXT_FILE_DIR);
        if (!dir.exists()) {
            dir.mkdirs();
        }
        dir = new File(BDBGUITestConfig.ENV_DIR + File.separator + DATA1_DIR);
        if (!dir.exists()) {
            dir.mkdirs();
        }
        dir = new File(BDBGUITestConfig.ENV_DIR + File.separator + DATA2_DIR);
        if (!dir.exists()) {
            dir.mkdirs();
        }
    }

}
