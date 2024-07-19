/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import java.net.URL;
import java.util.ResourceBundle;

import javafx.fxml.FXML;
import javafx.scene.control.TextArea;
import db_gui.BDBInitializable;
import javafx.scene.control.Button;
import javafx.scene.control.Tab;

/**
 * The root controller for the Environment Page.  It implements the feedback
 * box.
 */
public class EnvironmentPageController extends BDBInitializable {
    @FXML
    private TextArea environmentFeedbackBox;
    @FXML
    private Button envDisableButton;
    @FXML
    private Button envEnableButton;
    @FXML
    private Tab EnvironmentCreateTab;
    @FXML
    private Tab EnvironmentOpenTab;
    @FXML
    private Tab EnvironmentBackupTab;
    @FXML
    private Tab EnvironmentRecoveryTab;
    @FXML
    private Tab EnvironmentRemoveLogsTab;
    @FXML
    private Tab EnvironmentUpgradeTab;
    @FXML
    private Tab EnvironmentVerifyDataTab;
    @FXML
    private Tab EnvironmentVerifyLogTab;
    final static private String FXMLResource =
            "/db_gui/envpage/FXMLEnvironmentPage.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        bdbState.setFeedbackBox(environmentFeedbackBox);
        bdbState.clearTabs();
        bdbState.clearButtons();
        bdbState.setDisableButton(envDisableButton);
        bdbState.setEnableButton(envEnableButton);
        bdbState.enableFeedbackBox(true);
        bdbState.addTab(EnvironmentCreateTab);
        bdbState.addTab(EnvironmentOpenTab);
        bdbState.addTab(EnvironmentBackupTab);
        bdbState.addTab(EnvironmentRecoveryTab);
        bdbState.addTab(EnvironmentRemoveLogsTab);
        bdbState.addTab(EnvironmentUpgradeTab);
        bdbState.addTab(EnvironmentVerifyDataTab);
        bdbState.addTab(EnvironmentVerifyLogTab);
    }

    /**
     * Clears all the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        environmentFeedbackBox.clear();
    }

    /**
     * Gets the FXML file for the Environment Page, which is
     * envpage/FXMLEnvironmentPage.fxml.
     *
     * @return Returns the name of the FXML file for the Environment Page.
     */
    static public String getFXMLResource() {
        return FXMLResource;
    }
}
