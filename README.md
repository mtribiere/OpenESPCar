[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
![](/img/logo_large.png?raw=true "Logo")

# About the project
The aim of this project is to provide a fully-featured and generic plateform to create all kinds of ESP based RC car. 
From custom hardware to high-end API, everything is built from the ground-up.

# Current state
A first working POC has been made and tested successfully. However, there is still lots of things to do and maintenaing everything myself is not possible. 

# Hardware
For now on, <b>ESP12f-based</b> dev board, and <b>L298N</b>. However it should work with any other ESP board - H-bridge couple with minor changes.<br>
<br>
The overall architecture is pretty straight forward : 
 - The ESP board and motor drivers are placed inside the car
 - The ESP creates a Wifi AP and start-up a UDP server
 - The android smartphone connect to the Wifi AP
 - The App communicate starts a UDP client and communicate with the car
 
<br>
<img src="/img/diagram.png" alt="drawing" width="500"/>

# 🧰 PCB Design
The custom designed PCB for holding all the electronic
👉 ![See the doc](https://github.com/mtribiere/OpenESPCar/wiki/PCB-Design)

# 🤖 Embedded Firmware
The firmware flashed in the ESP-SOC
👉 ![See the doc](https://github.com/mtribiere/OpenESPCar/wiki/ESP-Firmware)

# 🔌 Protocol
The communication protocol used to control to car
👉 ![See the doc](https://github.com/mtribiere/OpenESPCar/wiki/Protocol)

# 📱 Android App
A direct implementation of the protocol for the POC
👉 ![See the doc](https://github.com/mtribiere/OpenESPCar/wiki/Android-App)
<br>
<img src="/img/android_screen.png" alt="drawing" width="200"/>
