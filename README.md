mDjiController
===============
 - Connect your DJI Remote Controller to your PC and use it to play simulators.
 - Currently confirmed working controllers: DJI Phantom 2
-----------------------------------------------------------------------------


This is a program that connects to your DJI Phantom Remote Controller (RC),
reads the stick positions and tells Windows that position. A simplified, wannabe driver.

In use:
![](https://raw.githubusercontent.com/Matsemann/mDjiController/master/in_use.png)

Playing LiftOff / Drone simulators with a DJI controller.

Installation / Usage
------------

In order for it to work, the original drivers from DJI must be installed, and something called vJoy.

* Download mDjiController.zip from above ( https://github.com/Matsemann/mDjiController/blob/master/mDjiController.zip?raw=true )
* Download DJI WIN Driver and DJI Assistant 2 ( only version 1.0.5 has been tested ) Installer from here: http://www.dji.com/product/phantom-2/download
* Download vJoy from here: http://vjoystick.sourceforge.net/site/index.php/download-a-install/72-download

  ! NOTE: DJI Assistant 2 and Vjoy apps do not need to be running to use this program. ( DJI Assistant cannot be running ).

* Install all three.
* Then connect your RC to your computer via USB and turn it on.

* Run mDjiController.exe
* For vJoy, write 1 unless you use vJoy already and have different configurations.

Then open a simulator and calibrate the controller.

How does it work?
-----------------

* It is possible to ask the controller for its status when it's connected. So this program continuously
	reads the status. It connects through COM. The output from the controller is just a list of numbers,
	but sampling many enough such lists it's easy to see a pattern for which number means what. The biggest issue
	was that the numbers are little endian encoded and uses two's complement, so it took some time to understand how
	each number behaved.

* vJoy is a virtual joystick that can be installed on your computer. Windows think it's a normal joystick. mDjiController
	takes the stick positions from the controller and tells Windows that this virtual joystick has the same positions.


TROUBLESHOOTING
---------------
* COM can't connect
	Open up device-manager.
	You should see a category named ( View < Show Hidden ) "Ports (COM & LPT) and then a device named "DJI USB Virtual COM (COMX)"
	If there is nothing there, the driver and Assistant from DJI are not installed correctly.

	The DJI Assistant Software does not need to be running, only installed, make sure they are not running, as this program
	cannot connect then.

* vJoy can't connect
	The error message should tell you if it's installed or not. Install the vJoy driver if it's not there.

* Not all sticks or buttons work
	*If they look correct in the log:*
	Your vJoy configuration may be wrong. Open "Configure vJoy" and reset configuration 1, or make a new one
	and tell mDjiController.exe to use that configuration.
	*If they are NOT correct in the log:*
	Your controller sends in a different format, so you need to figure that out, edit the code and recompile.
	(A bit advanced)
