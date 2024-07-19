/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import db_gui.envpage.BDBEnvInitializable;
import db_gui.envpage.UpgradeTask;

import java.io.File;
import java.net.URL;
import java.util.List;
import java.util.ResourceBundle;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.TextField;
import javafx.stage.FileChooser;

/**
 * EnvironmentUpgradeController implements the Upgrade tab of the Environment
 * Page.
 */
public class EnvironmentUpgradeController extends BDBEnvInitializable {
    @FXML
    private TextField UpgradeDatabasesTextField;
    private File[] databaseFiles;
    final static private String FXMLResource =
            "/db_gui/envpage/FXMLEnvironmentUpgradeTab.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        super.initialize(url, rb);
        databaseFiles = new File[0];
    }

    /**
     * Clears all the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        UpgradeDatabasesTextField.clear();
        databaseFiles = new File[0];
        super.clearAllFields();
    }

    /**
     * Handles the multi-file selector that selects database files to upgrade.
     * When new files are selected, they are display in the
     * UpgradeDatabasesTextField.
     *
     * @param event - Unused.
     */
    @FXML
    private void selectUpgradeDatabaseFiles(ActionEvent event) {
        FileChooser files = new FileChooser();
        List<File> dbFiles =
                files.showOpenMultipleDialog(bdbState.getMainStage());
        if (dbFiles != null && dbFiles.size() > 0) {
            databaseFiles = new File[0];
            databaseFiles = dbFiles.toArray(databaseFiles);
            String names = "";
            for (File dbFile : databaseFiles) {
                names = names + dbFile.getName() + " ";
            }
            UpgradeDatabasesTextField.setText(names);
        }
    }

    /**
     * Reads databases to upgrade from the UpgradeDatabaseTextField.
     *
     * @return - True on success, false on error.
     */
    private boolean getUpgradeDatabases() {
        String home = UpgradeDatabasesTextField.getText();
        if (home == null || home.isEmpty())
            return false;
        String[] dbs = home.split(",");
        databaseFiles = new File[dbs.length];
        int i = 0;
        for (String db : dbs) {
            db = db.trim();
            File file = new File(db);
            if (!file.isAbsolute()) {
                db = config.home + File.separator + db;
                file = new File(db);
            }
            if (!file.exists() || file.isDirectory()) {
                bdbState.printFeedback(
                        "Error, cannot find database:" + db);
                return false;
            }
            databaseFiles[i] = file;
            i++;
        }
        return true;
    }

    /**
     * Implements the Upgrade button.  Creates an UpgradeTask to perform the
     * upgrades in a separate thread.
     *
     * @param event - Unused.
     */
    @FXML
    private void handleUpgradeEnvironmentButton(ActionEvent event) {
        if (!getEncryptionKey())
            return;
        if (config.getHome() == null && !getEnvironmentHome()) {
            bdbState.printFeedback(
                    "Error, please select an Environment Home directory.");
            return;
        }
        if (databaseFiles.length < 1 && !getUpgradeDatabases()) {
            bdbState.printFeedback(
                    "Error, please select database files to upgrade.");
            return;
        }
        UpgradeTask upgrade = new UpgradeTask(config, databaseFiles);
        /* 
         * Disable the tabs so the user cannot change pages until the background
         * task is finished
         */
        bdbState.disable();
        new Thread(upgrade).start();
    }

    /**
     * Gets the FXML file for the Upgrade tab, which is
     * envpage/FXMLEnvironmentUpgradeTab.fxml.
     *
     * @return The FXML file for the Upgrade tab.
     */
    static public String getFXMLResource() {
        return FXMLResource;
    }
}
