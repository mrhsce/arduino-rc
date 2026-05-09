package com.mrhs.arduino.rc_controller;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import java.lang.reflect.Method;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Timer;
import java.util.TimerTask;

import io.github.controlwear.virtual.joystick.android.JoystickView;

public class MainActivity extends AppCompatActivity {

    JoystickView joystickVertical, joystickHorizontal;
    DatagramSocket udpSocket;
    InetAddress serverAddr;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        joystickVertical = findViewById(R.id.joystickView1);
        joystickHorizontal = findViewById(R.id.joystickView2);
        joystickVertical.setButtonDirection(1);
        joystickHorizontal.setButtonDirection(-1);

        try {
            udpSocket = new DatagramSocket(4320);
            serverAddr = InetAddress.getByName("192.168.4.1");
        } catch (Exception e) {
            e.printStackTrace();
        }


        new Timer().scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                int X = getX();
                int Y = getY();
                if (X != 0 || Y != 0) {
                    try {
                        String message = "X" + X + "Y" + Y;
                        byte[] buf = (message).getBytes();
                        DatagramPacket packet = new DatagramPacket(buf, buf.length, serverAddr, 4320);
                        udpSocket.send(packet);
                        Log.d("Time", " X: " + X);
                        Log.d("Time", " Y: " + Y);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }

            }
        }, 0, 100);//put here time 1000 milliseconds=1 second
    }

    public int getX() {
        try {
            Class<?> clazz = joystickVertical.getClass();
            Method getAngle = clazz.getDeclaredMethod("getAngle");
            Method getStrength = clazz.getDeclaredMethod("getStrength");
            getAngle.setAccessible(true);
            getStrength.setAccessible(true);
            Integer angle = (int) getAngle.invoke(joystickVertical);
            Integer strength = (int) getStrength.invoke(joystickVertical);
            switch (angle) {
                case 90:
                    return (int) Math.ceil(strength / 20);
                case 270:
                    return (int) Math.floor(-strength / 20);
                default:
                    return 0;
            }
        } catch (Exception exception) {
            return 0;
        }
    }

    public int getY() {
        try {
            Class<?> clazz = joystickHorizontal.getClass();
            Method getAngle = clazz.getDeclaredMethod("getAngle");
            Method getStrength = clazz.getDeclaredMethod("getStrength");
            getAngle.setAccessible(true);
            getStrength.setAccessible(true);
            Integer angle = (int) getAngle.invoke(joystickHorizontal);
            Integer strength = (int) getStrength.invoke(joystickHorizontal);
            switch (angle) {
                case 0:
                    return (int) Math.ceil(strength / 20);
                case 180:
                    return (int) Math.floor(-strength / 20);
                default:
                    return 0;
            }
        } catch (Exception exception) {
            return 0;
        }
    }
}