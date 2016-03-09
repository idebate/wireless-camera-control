wireless-camera-control
===========

Moteino based wireless control of (1) camera pan/tilt head and (2) zoom/focus/iris motor control via follow focus emulation.

Overview
--------

wireless-camera-control is a Moteino based standalone wireless controller for multiple pan/tilt heads based on the popular Bescor MP-101, along with provisions for controlling up to three geared motors for the implementation of zoom, follow focus and iris.

Requirements
------------

1. Moteino [USB preferred, but either would do] + RFM69 receiver ```http://www.lowpowerlab.com/```.
2. Bescor MP-101 pan/tilt head ```http://www.bescor.com/MP101.htm```.

Acknowledgements
----------------

1. Felix at LowPowerLab, for a great development platform.
2. B. Stott at the LowPowerLab forums, for the inspiration and the streamlined code examples.
3. Protechy, for the initial breakdown of the Bescor head, pinout diagrams and initial directions on PWM.

Future
------

Move beyond proof of concept for switch and potentiometer joystick for direction control. Next on line is implementation of speed control with potentiometer joystick.

Licence
-------

wireless-camera-control is licensed under the GPL version 3+.
