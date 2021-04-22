package com.mtribiere.carcontroller;

public interface CarCallbackInterface {
    void OnConnectionConfirmed();
    int getJoystickStrength();
    int getJoystickXPos();
    int getJoystickYPos();
}
