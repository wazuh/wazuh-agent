/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import java.io.IOException;
import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.CheckBox;
import javafx.scene.control.TextField;

/**
 * EnvironmentCreateController implements the Create tab of the Environment
 * page.
 */
public class EnvironmentCreateController extends BDBEnvInitializable {
    @FXML
    private TextField CacheSizeTextField;
    @FXML
    private CheckBox TransactionalCheckBox;
    final static private String FXMLResource
            = "/db_gui/envpage/FXMLEnvironmentCreateTab.fxml";

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
        TransactionalCheckBox.setSelected(false);
        super.clearAllFields();
    }

    /**
     * Gets the FXML file for the Create tab, which is
     * envpage/FXMLEnvironmentCreateTab.fxml.
     *
     * @return Returns the name of the FXML file for the Create tab.
     */
    static public String getFXMLResource() {
        return FXMLResource;
    }

    /**
     * Implements the Create button.  It creates the given environment, then
     * forwards to the Database Page.
     *
     * @param event - Unused.
     */
    @FXML
    private void handleCreateEnvironmentButton(ActionEvent event) {
        if (!getCacheSize(CacheSizeTextField) || !getEncryptionKey())
            return;
        if (config.getHome() == null && !getEnvironmentHome()) {
            bdbState.printFeedback(
                    "Error, please select an Environment Home directory.");
            return;
        }
        if (bdbState.openEnvironment(
                config, TransactionalCheckBox.isSelected(), true)) {
            bdbState.printFeedback("Environment successfully created.");
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
}
