package db_gui;

/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */

import java.io.IOException;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

/**
 * MainState is the class that launches the Berkeley Database GUI.
 */
public class db_gui extends Application {

    /**
     * Called when the JavaFX machine starts.  It loads the Environment Page and
     * creates the BDBState singleton that holds the Environment, Database,
     * Transaction, and Cursor.
     *
     * @param stage - The object used to display the GUI.
     * @throws IOException
     */
    @Override
    public void start(Stage stage) throws IOException {
        BDBState state = BDBState.getBDBState();
        state.setMainStage(stage);
        FXMLLoader loader = new FXMLLoader(getClass().getResource(
                "/db_gui/envpage/FXMLEnvironmentPage.fxml"));
        Parent root = loader.load();

        Scene scene = new Scene(root);

        stage.setScene(scene);
        stage.show();
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        launch(args);
    }

    /**
     * This is called when the JavaFX machine shuts down.  It closes cleanly
     * closes any open cursors, transactions, databases, and environment.
     */
    @Override
    public void stop() {
        BDBState state = BDBState.getBDBState();
        state.closeEnvironment();
    }

}
