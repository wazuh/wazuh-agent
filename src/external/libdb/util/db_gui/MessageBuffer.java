/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.BlockingQueue;

import javafx.animation.AnimationTimer;

/**
 * Collects all output messages from the GUI and BDB and collects them in a
 * buffer.  When the GUI refreshes it prints all the messages to the feedback
 * box.  This is done because printing each message out as soon as it arrives
 * can cause the JavaFX engine to be overwhelmed and crash.
 */
public class MessageBuffer extends AnimationTimer {
    private final BlockingQueue<String> messageBuffer;

    /**
     * Message buffer constructor.
     *
     * @param messageBuffer - A concurrent blocking queue that holds the
     * messages.
     */
    public MessageBuffer(
            BlockingQueue<String> messageBuffer) {
        this.messageBuffer = messageBuffer;
    }

    /**
     * This function is called by JavaFX whenever it refreshes the GUI.  It
     * dumps all the messages in the buffer into the feedback box.
     *
     * @param now - no used.
     */
    @Override
    public void handle(long now) {
        BDBState bdbState = BDBState.getBDBState();
        List<String> messages = new ArrayList<>();
        messageBuffer.drainTo(messages);
        if (!messages.isEmpty())
            bdbState.appendText(String.join("", messages));
    }
}
