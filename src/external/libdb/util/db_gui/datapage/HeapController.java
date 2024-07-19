/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.datapage;

import db_gui.datapage.BDBDataInitializable;
import com.sleepycat.db.DatabaseEntry;

import java.net.URL;
import java.util.ResourceBundle;

/**
 * Implements the heap Put and Browse data panels of the Data Access Page.
 */
public class HeapController extends BDBDataInitializable {
    private DatabaseEntry currentGetKey;
    final static private String FXMLResource = "/db_gui/datapage/FXMLHeap.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        super.initialize(url, rb);
    }

    /**
     * Clears the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        currentGetKey = null;
        super.clearAllFields();
    }

    /**
     * New heap records are appended to the database, so no key is defined when
     * adding one to the database.
     *
     * @return - An empty DatabaseEntry object.
     */
    @Override
    public DatabaseEntry getPutKeyDatabaseEntry() {
        return new DatabaseEntry();
    }

    /**
     * The heap key is not user readable, so it is not displayed.
     *
     * @param key - Unused.
     */
    @Override
    public void setPutKeyDatabaseEntry(DatabaseEntry key) {
        // no-op
    }

    /**
     * Users cannot enter keys for the database when using a heap, so the
     * Get functionality is not provided on the Heap Data page.  If users want
     * to browse Heap data they can only use the cursor Next and Previous
     * functions, and the current key is stored in the GetKey field.
     *
     * @return - The DatabaseEntry for the current key.
     */
    @Override
    public DatabaseEntry getGetKeyDatabaseEntry() {
        return currentGetKey;
    }

    /**
     * Users cannot enter keys for the database when using a heap, so the
     * Get functionality is not provided on the Heap Data page.  If users want
     * to browse Heap data they can only use the cursor Next and Previous
     * functions, and the current key is stored in the GetKey field.
     *
     * @param key - The DatabaseEntry of the current Get key.
     */
    @Override
    public void setGetKeyDatabaseEntry(DatabaseEntry key) {
        currentGetKey = key;
    }

    /**
     * Whether the current Get key is set or not.
     *
     * @return True if the Get key is set, false otherwise.
     */
    @Override
    public boolean getGetKeySet() {
        return currentGetKey != null;
    }

    /**
     * Whether to append or put new records into the database.
     *
     * @return - True, since heap records are appended to the database.
     */
    @Override
    public boolean getAppendRecords() {
        return true;
    }

}
