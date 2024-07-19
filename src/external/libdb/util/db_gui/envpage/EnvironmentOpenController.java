/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.net.URL;
import java.util.ResourceBundle;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.TextField;

/**
 * EnvironmentOpenController implements the Open tab of the Environment
 * page.
 */
public class EnvironmentOpenController extends BDBEnvInitializable {
    @FXML
    private TextField CacheSizeTextField;
    @FXML
    private TextField DataDirectoriesTextField;
    @FXML
    private TextField LogDirectoryTextField;
    @FXML
    private TextField ExtFileDirectoryTextField;
    final static private String FXMLResource =
            "/db_gui/envpage/FXMLEnvironmentOpenTab.fxml";

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
        CacheSizeTextField.clear();
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
     * Implements the Open button.  It opens the given environment, then
     * forwards to the Database Page.
     *
     * @param event - Unused.
     */
    @FXML
    private void handleOpenEnvironmentButton(ActionEvent event) {
        if (!getCacheSize(CacheSizeTextField) || !getEncryptionKey())
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
        if (bdbState.openEnvironment(config, false, false)) {
            bdbState.printFeedback("Environment successfully opened.");
            try {
                handleDatabasePageButton(null);
            } catch (IOException ex) {
                Logger.getLogger(
                        EnvironmentCreateController.class.getName()).log(
                        Level.SEVERE, null, ex);
                bdbState.printFeedback(
                        "Error forwarding to the database page.");
                bdbState.closeEnvironment();
            }
        }
    }

    /**
     * Gets the FXML file for the Open tab, which is
     * envpage/FXMLEnvironmentOpenTab.fxml.
     *
     * @return Returns the name of the FXML file for the Open tab.
     */
    static public String getFXMLResource() {
        return FXMLResource;
    }
}
