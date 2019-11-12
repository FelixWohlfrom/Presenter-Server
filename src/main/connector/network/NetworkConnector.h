/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Presenter. Server software to remote control a presentation.         *
 *  Copyright (C) 2019 Felix Wohlfrom                                    *
 *                                                                       *
 *  This program is free software: you can redistribute it and/or modify *
 *  it under the terms of the GNU General Public License as published by *
 *  the Free Software Foundation, either version 3 of the License, or    *
 *  (at your option) any later version.                                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * NetworkConnector.h
 *
 *  Created on: 06.01.2019
 *      Author: Felix Wohlfrom
 */

#ifndef SRC_MAIN_CONNECTOR_NETWORKCONNECTOR_H_
#define SRC_MAIN_CONNECTOR_NETWORKCONNECTOR_H_

#include "../RemoteControl.h"

#include <QTimer>
#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>

/**
 * A remote control class to control the presentation via network.
 * It broadcasts regularly a message that can be received from the
 * clients and can be used to automatically connect to our server.
 */
class NetworkConnector: public RemoteControl
{
    Q_OBJECT

public:
    /**
     * Creates a new network connector.
     */
    NetworkConnector();

    /**
     * Cleans up the network connector.
     */
    ~NetworkConnector();

    /**
     * Starts a new server.
     */
    void startServer();

    /**
     * Stops the running server.
     */
    void stopServer();

    /**
     * The network port on which broadcast messages will be sent.
     */
    static const int broadcastPort;

private:
    /**
     * Stores the message to be broadcasted to make the clients
     * aware of our server.
     */
    QByteArray broadcastMessage;

    /**
     * Broadcast timer to send the messages regularly.
     */
    QTimer broadcastTimer;

    /**
     * Udp socket to broadcast the presenter server availability.
     */
    QUdpSocket* broadcastSocket;

    /**
     * The tcp server for key command transmission.
     */
    QTcpServer* keyCommandServer;

    /**
     * The connected clients.
     */
    QList<QTcpSocket*> clientSockets;

    /**
     * Write a given message to the connected client.
     *
     * @param message The message to write.
     */
    void write(const QString& message);

private slots:
    /**
     * Method to emit the presenter broadcast message.
     */
    void broadcastServerAvailablility();

    /**
     * Called if a new client connected.
     */
    void clientConnected();

    /**
     * Called if a client disconnected.
     */
    void clientDisconnected();

    /**
     * Called if new data is available to read.
     */
    virtual void readSocket();
};

#endif /* SRC_MAIN_CONNECTOR_NETWORKCONNECTOR_H_ */
