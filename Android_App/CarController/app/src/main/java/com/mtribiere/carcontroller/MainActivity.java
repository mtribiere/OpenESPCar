package com.mtribiere.carcontroller;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.net.SocketException;

import io.github.controlwear.virtual.joystick.android.JoystickView;

public class MainActivity extends AppCompatActivity implements CarCallbackInterface {

    private EditText ipTextBar;
    private Button connectButton;
    private SeekBar seekBar;
    private TextView valueText;

    private Thread runnerThread;
    private UDPClient carConnection;

    private int joystickStrength = 0;
    private int joystickXPos = 50;
    private int joystickYPos = 50;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //Find the UI elements
        ipTextBar = (EditText) findViewById(R.id.ipTextBar);
        connectButton = (Button) findViewById(R.id.connectButton);

        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                try {

                    //Hide the keyboard
                    hideKeybaord(v);
                    
                    //Stop any previous communication
                    if(carConnection != null)
                        carConnection.close();

                    if(runnerThread != null)
                        runnerThread.join();

                    //Start the car communication
                    carConnection = new UDPClient(MainActivity.this, ipTextBar.getText().toString(),8888);
                    runnerThread = new Thread(carConnection);
                    runnerThread.start();

                } catch (SocketException | InterruptedException e) {
                    e.printStackTrace();
                    return;
                } catch (IOException e) {
                    e.printStackTrace();
                }

            }
        });

        //Getting the joystick position
        ((JoystickView) this.findViewById(R.id.joystick)).setOnMoveListener(new JoystickView.OnMoveListener() {
            @Override
            public void onMove(int angle, int strength) {
                joystickXPos =  ((JoystickView) findViewById(R.id.joystick)).getNormalizedX();
                joystickYPos =  ((JoystickView) findViewById(R.id.joystick)).getNormalizedY();
                joystickStrength = strength;
            }
        });

    }

    public int getJoystickStrength(){
        return joystickStrength;
    }

    public int getJoystickXPos(){
        return joystickXPos;
    }

    public int getJoystickYPos(){return joystickYPos;}

    private void hideKeybaord(View v) {
        InputMethodManager inputMethodManager = (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
        inputMethodManager.hideSoftInputFromWindow(v.getApplicationWindowToken(),0);
    }

    @Override
    public void OnConnectionConfirmed() {

        //Enable joystick UI controls
        ((JoystickView) this.findViewById(R.id.joystick)).setEnabled(true);

        //Display Toast
        this.runOnUiThread(new Runnable() {
            public void run() {
                Toast.makeText(getApplicationContext(),"Connection successfull",Toast.LENGTH_SHORT).show();
            }
        });

    }
}