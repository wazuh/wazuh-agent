/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import db_gui.envpage.BDBEnvInitializable;
import db_gui.envpage.RemoveLogTask;

import java.net.URL;
import java.util.ResourceBundle;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.TextField;

/**
 * EnvironmentRemoveLogController implements the Remove Old Logs tab of the
 * Environment Page.
 */
public class EnvironmentRemoveLogController extends BDBEnvInitializable {
    @FXML
    private TextField DataDirectoriesTextField;
    @FXML
    private TextField LogDirectoryTextField;
    @FXML
    private TextField ExtFileDirectoryTextField;
    final static private String FXMLResource =
            "/db_gui/envpage/FXMLEnvironmentRemoveLogTab.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        super.initialize(url, rb);
    }

    /**
     * Clears all the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        DataDirectoriesTextField.clear();
        LogDirectoryTextField.clear();
        ExtFileDirectoryTextField.clear();
        super.clearAllFields();
    }

    /**
     * Handles the Log directory chooser.  When a new directory
     * is chosen, it is displayed in LogDirectoryTextField.
     *
     * @param event - Unused.
     */
    @FXML
    private void selectLogDirectory(ActionEvent event) {
        selectLogDirectory(LogDirectoryTextField);
    }

    /**
     * Handles the External File directory chooser.  When a new directory
     * is chosen, it is displayed in ExtFileDirectoryTextField.
     *
     * @param event - Unused.
     */
    @FXML
    private void selectExternalFileDirectory(ActionEvent event) {
        selectExternalFileDirectory(ExtFileDirectoryTextField);
    }

    /**
     * Handles the Data Directories directory chooser.  When a new directory
     * is chosen, it is appended to the text displayed in
     * DataDirectoryTextField.
     *
     * @param event - Unused.
     */
    @FXML
    private void selectDataDirectories(ActionEvent event) {
        selectDataDirectories(DataDirectoriesTextField);
    }

    /**
     * Implements the Remove Logs button.  It creates a new RemoveLogTask that
     * performs the remove logs operation in a background thread.
     *
     * @param event
     */
    @FXML
    private void handleRemoveLogsEnvironmentButton(ActionEvent event) {
        if (!getEncryptionKey())
            return;
        if (config.getHome() == null && !getEnvironmentHome()) {
            bdbState.printFeedback(
                    "Error, please select an Environment Home directory.");
            return;
        }
        if (config.getDataDirs() == null &&
                !getDataDirectories(DataDirectoriesTextField))
            return;
        if (config.getExternalDir() == null &&
                !getExtFileDirectory(ExtFileDirectoryTextField))
            return;
        if (config.getLogDir() == null &&
                !getLogDirectory(LogDirectoryTextField))
            return;
        RemoveLogTask remove = new RemoveLogTask(config);
        /* 
         * Disable the tabs so the user cannot change pages until the background
         * task is finished
         */
        bdbState.disable();
        new Thread(remove).start();
    }

    /**
     * Gets the FXML file for the Remove Logs tab, which is
     * envpage/FXMLEnvironmentRemoveLogTab.fxml.
     *
     * @return Returns the name of the FXML file for the Remove Logs tab.
     */
    static public String getFXMLResource() {
        return FXMLResource;
    }
}
