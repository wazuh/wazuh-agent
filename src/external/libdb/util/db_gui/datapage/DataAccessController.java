/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.datapage;

import com.sleepycat.db.DatabaseType;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.ResourceBundle;

import javafx.fxml.FXML;
import javafx.scene.control.TextArea;
import db_gui.BDBInitializable;

/**
 * The root controller for the Data Access Page.  It implements the feedback
 * box and constructs the Data Access page based on the database type and
 * whether the Environment is transactional or not.
 */
public class DataAccessController extends BDBInitializable {
    @FXML
    private TextArea FeedbackBox;
    private final boolean transactional;
    private final DatabaseType type;
    private String FXMLResource = "";
    final static private String FXMLheader =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
                    "\n" +
                    "<?import java.lang.*?>\n" +
                    "<?import java.util.*?>\n" +
                    "<?import javafx.scene.*?>\n" +
                    "<?import javafx.scene.control.*?>\n" +
                    "<?import javafx.scene.layout.*?>\n" +
                    "\n" +
                    "\n" +
                    "<AnchorPane id=\"AnchorPane\" prefHeight=\"600.0\" prefWidth=\"786.0\" xmlns:fx=\"http://javafx.com/fxml/1\" xmlns=\"http://javafx.com/javafx/8\">\n" +
                    "    <children>";
    final static private String FXMLfooter =
            "<Separator layoutX=\"0.0\" layoutY=\"400.0\" prefHeight=\"5.0\" prefWidth=\"786.0\" />\n" +
                    "      <Button fx:id=\"EnvironmentPageButton\" layoutX=\"19.0\" layoutY=\"565.0\" mnemonicParsing=\"false\" onAction=\"#handleEnvironmentPageButton\" text=\"Environment Page\" accessibleHelp=\"Closes the current environment and database and returns to the Environment page.\" accessibleText=\"Environment Page Button\" />\n" +
                    "      <Button fx:id=\"DatabasePageButton\" layoutX=\"173.0\" layoutY=\"565.0\" mnemonicParsing=\"false\" onAction=\"#handleDatabasePageButton\" text=\"Database Page\" accessibleHelp=\"Closes the current database and returns to the Database page.\" accessibleText=\"Database Page Button\" />\n" +
                    "       <Button fx:id=\"CloseButton\" layoutX=\"300.0\" layoutY=\"565.0\" mnemonicParsing=\"false\" onAction=\"#handleCloseButton\" text=\"Close\" accessibleHelp=\"Closes the environment and exits the application.\" accessibleText=\"Close button\"/>" +
                    "      <TextArea fx:id=\"FeedbackBox\" layoutX=\"24.0\" layoutY=\"425.0\" prefHeight=\"120.0\" prefWidth=\"786.0\" accessibleHelp=\"Displays feedback, such as error messages and operation progress.\" accessibleText=\"Feedback text box\"/>\n" +
                    "    </children>\n" +
                    "</AnchorPane>";
    final static private String FXMLBtreeHash =
            "<Pane layoutX=\"0.0\" layoutY=\"50.0\" prefHeight=\"400.0\" prefWidth=\"786.0\">" +
                    "<fx:include fx:id=\"BtreeHash\" source=\"/db_gui/datapage/FXMLBtreeHash.fxml\" />\n" +
                    "</Pane>";
    final static private String FXMLHeap =
            "<Pane layoutX=\"0.0\" layoutY=\"50.0\" prefHeight=\"400.0\" prefWidth=\"786.0\">" +
                    "<fx:include fx:id=\"Heap\" source=\"/db_gui/datapage/FXMLHeap.fxml\" />\n" +
                    "</Pane>";
    final static private String FXMLQueue =
            "<Pane layoutX=\"0.0\" layoutY=\"50.0\" prefHeight=\"400.0\" prefWidth=\"786.0\">" +
                    "<fx:include fx:id=\"Queue\" source=\"/db_gui/datapage/FXMLQueue.fxml\" />\n" +
                    "</Pane>";
    final static private String FXMLRecno =
            "<Pane layoutX=\"0.0\" layoutY=\"50.0\" prefHeight=\"400.0\" prefWidth=\"786.0\">" +
                    "<fx:include fx:id=\"Recno\" source=\"/db_gui/datapage/FXMLRecno.fxml\" />\n" +
                    "</Pane>";
    final static private String FXMLTransaction =
            "<Pane layoutX=\"0.0\" layoutY=\"0.0\" prefHeight=\"50.0\" prefWidth=\"786.0\">" +
                    "<fx:include fx:id=\"Transaction\" source=\"/db_gui/datapage/FXMLTransaction.fxml\" />\n" +
                    "</Pane>";

    /**
     * Default constructor.  It constructs the FXML used to display the Data
     * Access page based on the database type and whether the Environment is
     * transactional or not.
     *
     * @param txn - Whether the Environment is transactional or not.
     * @param dbType - The database access method.
     */
    public DataAccessController(boolean txn, DatabaseType dbType) {
        transactional = txn;
        type = dbType;
        /* Contstruct the FXML document. */
        FXMLResource = FXMLheader;
        // Add transaction section if supported.
        if (transactional)
            FXMLResource += FXMLTransaction;
        // Set the access type dependent data sections.
        if (type == DatabaseType.BTREE || type == DatabaseType.HASH)
            FXMLResource += FXMLBtreeHash;
        else if (type == DatabaseType.HEAP)
            FXMLResource += FXMLHeap;
        else if (type == DatabaseType.QUEUE)
            FXMLResource += FXMLQueue;
        else if (type == DatabaseType.RECNO)
            FXMLResource += FXMLRecno;
        // Add the feedback box and environment and database buttons
        FXMLResource += FXMLfooter;
    }

    /**
     * Clears the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        FeedbackBox.clear();
    }

    /**
     * Returns the constructed FXML page as an InputStream.
     *
     * @return
     */
    public InputStream getFXMLInputStream() {
        return new ByteArrayInputStream(
                FXMLResource.getBytes(StandardCharsets.UTF_8));
    }

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        bdbState.setFeedbackBox(FeedbackBox);
        bdbState.clearButtons();
        bdbState.clearTabs();
    }


}
