/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import db_gui.envpage.BDBEnvInitializable;
import db_gui.envpage.VerifyLogTask;
import com.sleepycat.db.LogVerifyConfig;

import java.io.File;
import java.net.URL;
import java.util.ResourceBundle;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.TextField;
import javafx.stage.DirectoryChooser;

/**
 * EnvironmentVerifyLogController controls the Verify Log tap of the Environment
 * Page.
 */
public class EnvironmentVerifyLogController extends BDBEnvInitializable {
    @FXML
    private TextField LogDirectoryTextField;
    @FXML
    private TextField CacheSizeTextField;
    @FXML
    private TextField TempDirectoryTextField;
    private File tempHome;
    final static private String FXMLResource =
            "/db_gui/envpage/FXMLEnvironmentVerifyLogTab.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        super.initialize(url, rb);
    }

    /**
     * Clears the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        LogDirectoryTextField.clear();
        CacheSizeTextField.clear();
        TempDirectoryTextField.clear();
        tempHome = null;
        super.clearAllFields();
    }

    /**
     * Handles the Temporary Environment Home directory chooser.  When a
     * new directory is chosen it is displayed in the TempDirectoryTextField.
     *
     * @param event - Unused.
     */
    @FXML
    private void selectTempEnvironmentHome(ActionEvent event) {
        DirectoryChooser tempHomeDir = new DirectoryChooser();
        File home = tempHomeDir.showDialog(bdbState.getMainStage());
        if (home != null) {
            tempHome = home;
            TempDirectoryTextField.setText(home.getAbsolutePath());
        }
    }

    /**
     * Handles the Log Directory directory chooser.  When a new directory is
     * chosen it is displayed in the LogDirectoryTextField.
     *
     * @param event - Unused.
     */
    @FXML
    private void selectLogDirectory(ActionEvent event) {
        selectLogDirectory(LogDirectoryTextField);
    }

    /**
     * Reads the text in TempDirectoryTextField and converts it to a directory
     * sets it as the temporary home directory..
     *
     * @return True on success, false on error.
     */
    public boolean getTempEnvironmentHome() {
        String home = TempDirectoryTextField.getText();
        if (home == null || home.isEmpty())
            return true;
        File file = new File(home);
        if (!file.exists() || !file.isDirectory())
            return false;
        tempHome = file;
        return true;
    }

    /**
     * Handles the Verify Logs button.  It creates a new LogVerifyTask to
     * perform the log verify as a background thread.
     *
     * @param event
     */
    @FXML
    private void handleVerifyLogsEnvironmentButton(ActionEvent event) {
        int tempCacheSize;
        if (!getCacheSize(CacheSizeTextField) || !getEncryptionKey())
            return;
        tempCacheSize = (new Long(config.getCacheSize())).intValue();
        if (config.getHome() == null && !getEnvironmentHome()) {
            bdbState.printFeedback(
                    "Error, please select an Environment Home directory.");
            return;
        }
        LogVerifyConfig logConfig = new LogVerifyConfig();
        logConfig.setVerbose(true);
        if (tempHome == null && !getTempEnvironmentHome()) {
            bdbState.printFeedback(
                    "Error, cannot find the temporary home directory.");
            return;
        }
        if (tempHome != null)
            logConfig.setEnvHome(tempHome.getAbsolutePath());
        if (tempCacheSize > 0)
            logConfig.setCacheSize(tempCacheSize);
        if (config.getLogDir() == null &&
                !getLogDirectory(LogDirectoryTextField))
            return;
        VerifyLogTask verify = new VerifyLogTask(config, logConfig);
        /* 
         * Disable the tabs so the user cannot change pages until the background
         * task is finished
         */
        bdbState.disable();
        new Thread(verify).start();
    }

    /**
     * Gets the FXML file for the Verify Log tab, which is
     * envpage/FXMLEnvironmentVerifyLogTab.fxml.
     *
     * @return The FXML file for the Verify Log tab.
     */
    static public String getFXMLResource() {
        return FXMLResource;
    }
}
