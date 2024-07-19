/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.datapage;

import com.sleepycat.db.DatabaseException;

import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.Button;
import db_gui.BDBInitializable;

/**
 * TransactionController handles the transaction panel of the Data Access Page.
 */
public class TransactionController extends BDBInitializable {
    @FXML
    private Button TxnBeginButton;
    @FXML
    private Button TxnCommitButton;
    @FXML
    private Button TxnAbortButton;
    final static private String FXMLResource =
            "/db_gui/datapage/FXMLTransaction.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        // no-op
    }

    /**
     * Clears the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        /*
         * no-op - The user has to explicitly decide if they want to commit
         * or abort the transaction.
         */
    }

    /**
     * Begins a transaction, disables the Begin button, and
     * enables the Commit and Abort buttons.
     *
     * @param event - Unused.
     */
    @FXML
    private void handleTxnBeginButton(ActionEvent event) {
        try {
            bdbState.beginTxn();
            TxnBeginButton.setDisable(true);
            TxnCommitButton.setDisable(false);
            TxnAbortButton.setDisable(false);
        } catch (DatabaseException ex) {
            Logger.getLogger(
                    TransactionController.class.getName()).log(
                    Level.SEVERE, null, ex);
            bdbState.printFeedback(
                    "Error creating transaction: " + ex.getMessage());
        }
    }

    /**
     * Commits a transaction, enables the Begin button, and
     * disables the Commit and Abort buttons.  Note that the
     * commitTransaction function exits the program if the
     * operation fails.
     *
     * @param event - Unused.
     */
    @FXML
    private void handleTxnCommitButton(ActionEvent event) {
        bdbState.commitTransaction();
        TxnBeginButton.setDisable(false);
        TxnCommitButton.setDisable(true);
        TxnAbortButton.setDisable(true);
    }

    /**
     * Aborts a transaction, enables the Begin button, and
     * disables the Commit and Abort buttons.  Note that the
     * abortTransaction function exits the program if the
     * operation fails.
     *
     * @param event - Unused.
     */
    @FXML
    private void handleTxnAbortButton(ActionEvent event) {
        bdbState.abortTransaction();
        TxnBeginButton.setDisable(false);
        TxnCommitButton.setDisable(true);
        TxnAbortButton.setDisable(true);
    }

}
