/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui;

import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.DatabaseType;
import db_gui.datapage.DataAccessController;
import db_gui.dbpage.DatabasePageController;
import db_gui.envpage.EnvironmentPageController;

import java.io.IOException;

import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

/**
 * BDBInitializable is the base class of all controller classes of the GUI.
 * It contains the BDBState object and implements common buttons such as the
 * Clear button and the buttons that return the user to the Environment Page
 * and the Database Page.
 */
public abstract class BDBInitializable implements Initializable {
    protected final BDBState bdbState;

    /**
     * Default constructor, sets the BDBState.
     */
    public BDBInitializable() {
        bdbState = BDBState.getBDBState();
    }

    /**
     * Clears all the fields in the GUI.
     */
    public abstract void clearAllFields();

    /**
     * Gets the name of the FXML file for the current GUI page.
     *
     * @return The name of the FXML file for the current GUI page.
     */
    static public String getFXMLResource() {
        return null;
    }

    /**
     * Gets the BDBState object.
     *
     * @return The BDBStage object.
     */
    public BDBState getBdbState() {
        return bdbState;
    }

    /**
     * Implements the Close button by cleanly shutting down the JavaFX
     * application.
     *
     * @param event - Unused.
     */
    @FXML
    protected void handleCloseButton(ActionEvent event) {
        Platform.exit();
    }
    
    /**
     * Implements the Enable Feedback button by setting
     * the feedbackBox state to true.
     *
     * @param event - Unused.
     */
    @FXML
    protected void handleEnableFeedbackBox(ActionEvent event) {
        bdbState.enableFeedbackBox(true);
    }
    
    /**
     * Implements the Disable Feedback button by setting
     * the feedbackBox state to true.
     *
     * @param event - Unused.
     */
    @FXML
    protected void handleDisableFeedbackBox(ActionEvent event) {
        bdbState.enableFeedbackBox(false);
    }

    /**
     * Implement the Return to Environment Page button.
     *
     * @param event - Unused.
     * @throws IOException
     */
    @FXML
    protected void handleEnvironmentPageButton(
            ActionEvent event) throws IOException {
        FXMLLoader loader = new FXMLLoader(getClass().getResource(
                EnvironmentPageController.getFXMLResource()));
        Parent root = loader.load();

        Stage mainStage = bdbState.getMainStage();

        bdbState.closeEnvironment();
        Scene scene = new Scene(root);

        mainStage.setScene(scene);
        mainStage.show();
    }

    /**
     * Implement the Return to Database Page button.
     *
     * @param event - Unused.
     * @throws IOException
     */
    @FXML
    protected void handleDatabasePageButton(
            ActionEvent event) throws IOException {
        bdbState.closeDatabase();
        FXMLLoader loader = new FXMLLoader(getClass().getResource(
                DatabasePageController.getFXMLResource()));
        Parent root = loader.load();

        Stage mainStage = bdbState.getMainStage();

        Scene scene = new Scene(root);

        mainStage.setScene(scene);
        mainStage.show();
    }

    /**
     * Implements the Clear button by called the implementation of the
     * clearAllFields function in the inheriting classes.
     *
     * @param event - Unused.
     */
    @FXML
    protected void handleClearKey(ActionEvent event) {
        clearAllFields();
    }

    /**
     * Changes the current GUI page to the Data Access Page.
     *
     * @param bdbState - The state of the Berkeley DB environment.
     * @param type - The type of the current database.
     * @throws IOException
     */
    public static Void forwardDataAccessPage(
            BDBState bdbState, DatabaseType type) throws IOException {
        FXMLLoader loader = new FXMLLoader();
        DataAccessController dac =
                new DataAccessController(bdbState.isTransactional(), type);
        loader.setController(dac);
        loader.setLocation(dac.getClass().getResource("FXMLDataAccess.fxml"));
        Parent root = loader.load(dac.getFXMLInputStream());

        Stage mainStage = bdbState.getMainStage();
        Scene scene = new Scene(root);

        mainStage.setScene(scene);
        mainStage.show();
        return null;
    }

    /**
     * Handles a DealockException error by aborting the current transaction.
     *
     * @return True of the deadlock was successfully resolved, otherwise false.
     */
    public static boolean handleDeadlockException() {
        BDBState bdbState = BDBState.getBDBState();
        bdbState.printFeedback("Error: operation failed due to deadlock.");
        if (bdbState.getTxn() != null) {
            bdbState.abortTransaction();
            try {
                bdbState.beginTxn();
            } catch (DatabaseException ex) {
                bdbState.printFeedback(
                        "Error restarting transaction, please exit program and run recovery.");
                return false;
            }
            bdbState.printFeedback(
                    "The transaction was aborted to fix the deadlock " +
                            "and a new transaction was created.");
            bdbState.printFeedback("Please retry operation.");
        }
        return true;
    }
}
