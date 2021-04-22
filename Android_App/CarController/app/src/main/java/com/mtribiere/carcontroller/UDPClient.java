package com.mtribiere.carcontroller;

import android.app.Activity;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;

import io.github.controlwear.virtual.joystick.android.JoystickView;

public class UDPClient implements Runnable {

    private static final int STRENGTH_DEAD_ZONE = 30;
    private static final int DIRECTION_DEAD_ZONE = 10;

    private CarCallbackInterface carCallback;

    private DatagramSocket socket;
    private InetSocketAddress socketAddress;

    JoystickView joystick;

    private boolean isConnected = false;
    private int failCount = 0;


    public UDPClient(Activity _carCallback, String _ip, int _port) throws IOException {

        System.out.println("[CarController] Client start");

        this.joystick = (JoystickView) _carCallback.findViewById(R.id.joystick);
        this.carCallback = (CarCallbackInterface) _carCallback;

        this.socket = new DatagramSocket();
        this.socket.setSoTimeout(1000);
        socketAddress = new InetSocketAddress(_ip, _port);

    }

    public void run() {

        //Connection routine
        DatagramPacket dataReceived = new DatagramPacket(new byte[50], 50);
        byte[] connectionRequest = new byte[]{0x01, (byte) 'H', (byte) 'E', (byte) 'L', (byte) 'L', (byte) 'O'};
        DatagramPacket connectionRequestPacket = new DatagramPacket(connectionRequest, connectionRequest.length, socketAddress);

        while(!isConnected) {

            try {

                //Send the request
                socket.send(connectionRequestPacket);
                socket.receive(dataReceived);

                System.out.println(dataReceived.getData()[0]);

                //Check the response
                if(dataReceived.getData()[0] == 0x02 && dataReceived.getData()[1] == 'O' && dataReceived.getData()[2] == 'K')
                    //The response is correct
                    isConnected = true;


            } catch (IOException e) {
                System.out.println("[CarController] Request timed out, retrying....");
            }

        }

        //Connection is confirmed
        System.out.println("[CarController] Connection was sucessfull");
        this.carCallback.OnConnectionConfirmed();

        byte[] bufE = new byte[4];
        bufE[0] = 0x03;

        //Send move commands
        while(isConnected) {

            //Process the force value
            if(carCallback.getJoystickStrength() < STRENGTH_DEAD_ZONE || (carCallback.getJoystickYPos() <= 50+DIRECTION_DEAD_ZONE && carCallback.getJoystickYPos() >= 50-DIRECTION_DEAD_ZONE))
                bufE[1] = 0;
            else
                bufE[1] = (byte) carCallback.getJoystickStrength();

            //Process the direction
            // 0 : Forward
            // 1 : Backward
            bufE[2] = (byte) (carCallback.getJoystickYPos() <= 50 ? 0x00 : 0x01);

            //Process the steering
            // 0 : Neutral
            // 1 : Right
            // 2 : Left
            if(carCallback.getJoystickXPos() <= 50-DIRECTION_DEAD_ZONE)
                bufE[3] = 0x02;
            else if(carCallback.getJoystickXPos() >= 50+DIRECTION_DEAD_ZONE)
                bufE[3] = 0x01;
            else
                bufE[3] = 0x00;

            DatagramPacket dpE = new DatagramPacket(bufE, bufE.length, socketAddress);

            try {

                socket.send(dpE);
                Thread.sleep(50);

            } catch (IOException | InterruptedException e) {
                e.printStackTrace();
                failCount++;

                if(failCount >= 20)
                    isConnected = false;

            }

        }

    }


    public void close() throws InterruptedException {

        //Declare the connection closed
        this.isConnected = false;

        //Close the socket
        if(socket != null)
            socket.close();
    }

    private static double mapRange(double a1, double a2, double b1, double b2, double s){
        return b1 + ((s - a1)*(b2 - b1))/(a2 - a1);
    }

}
