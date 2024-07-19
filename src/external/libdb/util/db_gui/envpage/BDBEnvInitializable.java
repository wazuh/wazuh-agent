/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import java.io.File;
import java.net.URL;
import java.util.ResourceBundle;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.TextField;
import javafx.stage.DirectoryChooser;
import db_gui.BDBInitializable;
import db_gui.BDBState;

/**
 * BDBEnvInitializeable is the parent class of all the controllers of the
 * Environment Page.  It implements operations that are part of all or most of
 * each environment tab, such as a field to enter an encryption key, and the
 * directory chooser for the environment home.
 */
class BDBEnvInitializable extends BDBInitializable {
    @FXML
    protected TextField DirectoryTextField;
    @FXML
    protected TextField EncryptionPasswordField;
    protected EnvConfig config;

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL location, ResourceBundle resources) {
        config = new EnvConfig();
    }

    /**
     * Clears all the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        DirectoryTextField.clear();
        if (EncryptionPasswordField != null)
            EncryptionPasswordField.clear();
        config = new EnvConfig();
    }

    /**
     * Implements the Environment Home directory chooser.  When a new directory
     * is chosen, it is displayed in the DirectoryTextField.
     *
     * @param event - Unused.
     */
    @FXML
    protected void selectEnvironmentHome(ActionEvent event) {
        DirectoryChooser homeDir = new DirectoryChooser();
        File home = homeDir.showDialog(bdbState.getMainStage());
        if (home != null) {
            config.setHome(home);
            DirectoryTextField.setText(home.getAbsolutePath());
        }
    }

    /**
     * Reads the text in DirectoryTextField and converts it to a directory and
     * sets it as the environment home in the EnvironmentConfig.
     *
     * @return True on success, false on error.
     */
    protected boolean getEnvironmentHome() {
        String home = DirectoryTextField.getText();
        if (home == null || home.isEmpty())
            return false;
        File file = new File(home);
        if (!file.exists() || !file.isDirectory())
            return false;
        config.setHome(file);
        return true;
    }

    /**
     * Implements the EncryptionPasswordField.
     *
     * @param event - Unused.
     */
    protected boolean getEncryptionKey() {
        String key = EncryptionPasswordField.getText();
        if (key != null && key.length() > 0)
            config.setEncryptionKey(key);
        return true;
    }

    /**
     * Implements the CacheSizeTextField.
     *
     * @param event - Unused.
     */
    protected boolean getCacheSize(TextField CacheSizeTextField) {
        String cacheString = CacheSizeTextField.getText();
        if (cacheString == null || cacheString.length() == 0)
            return true;
        try {
            long cacheBytes = Long.parseLong(cacheString);
            config.setCacheSize(cacheBytes);
        } catch (NumberFormatException ex) {
            bdbState.printFeedback(
                    "Invalid value for cache size: " + cacheString);
            CacheSizeTextField.clear();
            return false;
        }
        return true;
    }

    /**
     * Handles the Log directory chooser.  When a new directory
     * is chosen, it is displayed in LogDirectoryTextField.
     *
     * @param event - Unused.
     */
    protected void selectLogDirectory(TextField LogDirectoryTextField) {
        DirectoryChooser logDir = new DirectoryChooser();
        File dir = logDir.showDialog(bdbState.getMainStage());
        if (dir != null) {
            config.setLogDir(dir);
            LogDirectoryTextField.setText(dir.getAbsolutePath());
        }
    }

    /**
     * Reads the text in the test field and converts it to a directory and
     * sets it as the log directory in the EnvironmentConfig.
     *
     * @param LogDirectoryTextField - TextField in which to read the path to
     * the
     * log directory.
     * @return True on success, false on error.
     */
    protected boolean getLogDirectory(TextField LogDirectoryTextField) {
        String home = LogDirectoryTextField.getText();
        if (home == null || home.isEmpty())
            return true;
        home = home.trim();
        File file = new File(home);
        File origFile = file;
        if (!file.isAbsolute()) {
            home = config.home + File.separator + home;
            file = new File(home);
        }
        if (!file.exists() || !file.isDirectory()) {
            BDBState.getBDBState().printFeedback(
                    "Error, cannot find directory:" + home);
            return false;
        }
        config.setLogDir(origFile);
        return true;
    }

    /**
     * Handles the External File directory chooser.  When a new directory
     * is chosen, it is displayed in ExtFileDirectoryTextField.
     *
     * @param event - Unused.
     */
    protected void selectExternalFileDirectory(
            TextField ExtFileDirectoryTextField) {
        DirectoryChooser extDir = new DirectoryChooser();
        File dir = extDir.showDialog(bdbState.getMainStage());
        if (dir != null) {
            config.setExternalDir(dir);
            ExtFileDirectoryTextField.setText(dir.getAbsolutePath());
        }
    }

    /**
     * Reads the text in the test field and converts it to a directory and
     * sets it as the external file directory in the EnvironmentConfig.
     *
     * @param ExtFileDirectoryTextField - TextField in which to read the path
     * to
     * the external file directory.
     * @return - True on success, false on error.
     */
    protected boolean getExtFileDirectory(TextField ExtFileDirectoryTextField) {
        String home = ExtFileDirectoryTextField.getText();
        if (home == null || home.isEmpty())
            return true;
        home = home.trim();
        File file = new File(home);
        File origFile = file;
        if (!file.isAbsolute()) {
            home = config.home + File.separator + home;
            file = new File(home);
        }
        if (!file.exists() || !file.isDirectory()) {
            BDBState.getBDBState().printFeedback(
                    "Error, cannot find directory:" + home);
            return false;
        }
        config.setExternalDir(origFile);
        return true;
    }

    /**
     * Handles the Data Directories directory chooser.  When a new directory
     * is chosen, it is appended to the text displayed in
     * DataDirectoryTextField.
     *
     * @param event - Unused.
     */
    protected void selectDataDirectories(TextField DataDirectoriesTextField) {
        DirectoryChooser dataDirs = new DirectoryChooser();
        File dataDir = dataDirs.showDialog(bdbState.getMainStage());
        if (dataDir != null) {
            config.addDataDir(dataDir);
            File[] dirs = config.getDataDirs();
            String names = "";
            for (File dir : dirs) {
                names = names + dir.getName() + " ";
            }
            DataDirectoriesTextField.setText(names);
        }
    }

    /**
     * Reads the text in the test field and converts it to directories and
     * sets it as the data directory in the EnvironmentConfig.
     *
     * @param DataFileDirectoryTextField - TextField in which to read the paths
     * the data directories.
     * @return - True on success, false on error.
     */
    protected boolean getDataDirectories(TextField DataDirectoriesTextField) {
        String home = DataDirectoriesTextField.getText();
        if (home == null || home.isEmpty())
            return true;
        String[] dirs = home.split(",");
        for (String dir : dirs) {
            dir = dir.trim();
            File file = new File(dir);
            File origFile = file;
            if (!file.isAbsolute()) {
                dir = config.home + File.separator + dir;
                file = new File(dir);
            }
            if (!file.exists() || !file.isDirectory()) {
                BDBState.getBDBState().printFeedback(
                        "Error, cannot find directory:" + dir);
                return false;
            }
            config.addDataDir(origFile);
        }
        return true;
    }

}
