# My Ardiuno projects
Author: Vincent Lacasse <br/>
First commit on: 2021-12-22 (long due!) <br/>

<p>Version: 1.0.0</p>

<p>At first commit, it contained:</p>

<p>Ligthing effects for Martin Leduc's LuxSonarium project, including the 'sweep' suite. It is base on the Arduio UNO R3. Some where prototypes for Martin. Other versions where usefull to replace the PWM driver which caused problems (spikes in lights intensity during slow sweeps). It also was adapted for two PWM channels (the Uno has 2 different hardware PWM, I mean working differently). Finally, I put some kind of front end on the serial port to allowing Martin to try some stuff himself.</p>

<p>A signal processing project that processes a sound signal and find its fundamental frequency, which was successful. It is based on the Arduino Due. A switch to the Due was done to get more RAM since I needed a 2Kb FFT which requises 8Kb of RAM in my case. The FFTtest5_due_display is the last version that compiles and runs adequately. This particular version was written to test the H1632 display. The result of this test is: the H1632 display should not be used as it is generating noise on the audio side. Unknown harmonics where observed in the spectrum when it was turned on.  The FFTtest6_due_library was being developped at the time.  It's being adapted to use the Signal library, which is itself under test.</p>

<p>The rest are utilities and tests that were performed along that time.

<p>I made a change to the CmdArduino Library. No error message is outputed if an empty command line is entered.</p>



 
