/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import java.io.File;
import java.util.LinkedList;
import java.util.List;

/**
 * Contains common configurations used by many of the tabs
 * in the Environment Page.
 */
public class EnvConfig {
    File home;
    String encryptionKey;
    long cacheSize;
    List<File> dataDirs;
    File logDir;
    File externalDir;

    /**
     * Default constructor.
     */
    public EnvConfig() {
        home = null;
        encryptionKey = null;
        cacheSize = 0;
        dataDirs = null;
        logDir = null;
        externalDir = null;
    }

    /**
     * Get the environment home.
     *
     * @return - Environment home.
     */
    public File getHome() {
        return home;
    }

    /**
     * Get the encryption key.
     *
     * @return - Encryption key.
     */
    public String getEncryptionKey() {
        return encryptionKey;
    }

    /**
     * Get the cache size.
     *
     * @return - Cache size.
     */
    public long getCacheSize() {
        return cacheSize;
    }

    /**
     * Get the data directories.
     *
     * @return - Data directories.
     */
    public File[] getDataDirs() {
        if (dataDirs == null)
            return null;
        File[] dirs = new File[dataDirs.size()];
        dataDirs.toArray(dirs);
        return dirs;
    }

    /**
     * Get the logs directory.
     *
     * @return - Logs directory.
     */
    public File getLogDir() {
        return logDir;
    }

    /**
     * Get the external file directory.
     *
     * @return - External file directory.
     */
    public File getExternalDir() {
        return externalDir;
    }

    /**
     * Set the environment home.
     *
     * @param home - Environment home.
     */
    public void setHome(File home) {
        this.home = home;
    }

    /**
     * Set the encryption key.
     *
     * @param encryptionKey - Encryption key.
     */
    public void setEncryptionKey(String encryptionKey) {
        this.encryptionKey = encryptionKey;
    }

    /**
     * Set the cache size.
     *
     * @param cacheSize - Cache size.
     */
    public void setCacheSize(long cacheSize) {
        this.cacheSize = cacheSize;
    }

    /**
     * Add a data directory.
     *
     * @param dataDir - A data directory.
     */
    public void addDataDir(File dataDir) {
        if (dataDirs == null)
            dataDirs = new LinkedList<>();
        dataDirs.add(dataDir);
    }

    /**
     * Set the logs directory.
     *
     * @param logDir - Log directory.
     */
    public void setLogDir(File logDir) {
        this.logDir = logDir;
    }

    /**
     * Set the external file directory.
     *
     * @param externalDir - External file directory.
     */
    public void setExternalDir(File externalDir) {
        this.externalDir = externalDir;
    }

}
