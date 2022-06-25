# AutoGreenhouse
For my greenhouse, I built an automatic temperature control device. Depending on the performance of the DHS22 sensor, we either open the window with an actuator or close it to keep the temperature we need in the greenhouse.
To control the device and display logs, I connected it to Home Assistance ESP HOME.

To use the firmware, you should change it in DHS.yaml
wifi login and password
AP login and password
API key
ota pass

Also change the pins to yours in testESPhome.h

To add the DHSesp library to ESPhome, I had to rewrite it, remove the .cpp file and write the implementation in .h
This library was not written by me, all links can be found inside DHSesp.h

It is planned to add automatic watering for the greenhouse and control from the screen connected via psi
