Mavic Mini 1 Controller as Gamepad
===============
 - Connect your Mavic Mini 1 controller to your PC and use it to play simulators.
 - Currently confirmed working controllers: MR1SD25
-----------------------------------------------------------------------------

Credits
-----------------
This program is heavily based on projects:

* mDjiController https://github.com/Matsemann/mDjiController
* mDjiController.fork https://github.com/slaterbbx/mDjiController.fork
* MiniDjiController https://github.com/hjstn/miniDjiController

How does it work?
-----------------

* Mavic Mini 1 Controller is not recognized as Gamepad on windows. 
* It has COM port interface only.
* It is possible to ask the controller for its status. 
* So this program continuously reads the status and pass sticks and buttons state to vJoy.
* vJoy is a virtual joystick that can be installed on your computer. 
* Windows think it's a normal joystick. 


Installation / Usage
------------

* Download MavicMiniControllerAsGamepad.zip from Releases
* Download vJoy from here: https://sourceforge.net/projects/vjoystick/files/latest/download

* Install vJoy.
* Then connect your RC to your computer via USB and turn it on.
* Unzip MavicMiniControllerAsGamepad.zip to some folder
* Run MavicControllerAsGamepad.exe, watch console window, let it running.

Then open a simulator and calibrate the controller.


TROUBLESHOOTING
---------------
* COM can't connect
	Open up device-manager.
	You should see a category named ( View < Show Hidden ) "Ports (COM & LPT) and then a device named "DJI USB Virtual COM (COMX)"

* vJoy can't connect
	The error message should tell you if it's installed or not. Install the vJoy driver if it's not there.

* Not all sticks or buttons work
	*If they look correct in the log:*
	Your vJoy configuration may be wrong. Open "Configure vJoy" and reset configuration 1.
	*If they are NOT correct in the log:*
	Your controller sends in a different format, so you need to figure that out, edit the code and recompile.
	(A bit advanced)
