/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.dbpage;

import java.net.URL;
import java.util.ResourceBundle;

import javafx.fxml.FXML;
import javafx.scene.control.TextArea;
import db_gui.BDBInitializable;
import javafx.scene.control.Button;
import javafx.scene.control.Tab;

/**
 * The root controller for the Database Page.  It implements the feedback
 * box.
 */
public class DatabasePageController extends BDBInitializable {
    @FXML
    private TextArea dbFeedbackBox;
    @FXML
    private Button dbDisableButton;
    @FXML
    private Button dbEnableButton;
    @FXML
    private Tab DatabaseCreateTab;
    @FXML
    private Tab DatabaseOpenTab;
    @FXML
    private Tab DatabaseCompactTab;
    @FXML
    private Tab DatabaseRemoveTab;
    @FXML
    private Tab DatabaseRenameTab;
    @FXML
    private Button EnvironmentPageButton;
    final static private String FXMLResource
            = "/db_gui/dbpage/FXMLDatabasePage.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        bdbState.setFeedbackBox(dbFeedbackBox);
        bdbState.clearTabs();
        bdbState.addTab(DatabaseCreateTab);
        bdbState.addTab(DatabaseOpenTab);
        bdbState.addTab(DatabaseCompactTab);
        bdbState.addTab(DatabaseRemoveTab);
        bdbState.addTab(DatabaseRenameTab);
        bdbState.clearButtons();
        bdbState.setDisableButton(dbDisableButton);
        bdbState.setEnableButton(dbEnableButton);
        bdbState.enableFeedbackBox(true);
        bdbState.addButton(EnvironmentPageButton);
    }

    /**
     * Clears the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        dbFeedbackBox.clear();
    }

    /**
     * Gets the FXML file for the Database Page, which is
     * dbpage/FXMLDatabasePage.fxml.
     *
     * @return Returns the name of the FXML file for the Database Page.
     */
    static public String getFXMLResource() {
        return FXMLResource;
    }

}
