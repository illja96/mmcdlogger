# MMCd Datalogger
PalmOS automotive diagnostic and datalogging tool compatible with many pre-OBDII (1990-1994) Mitsubishi vehicles.

# About the Hardware
All following info has been verified on my Mitsubishi Galant E33A (4G63 SOHC), but it should apply on other pre-OBDII ECUs as well.
There's absolutely NO WARRANTY on info and software, you can use it at your own risk.
How to make your own datalogging cable...

ALDL diagnostic connector:

![ALDL diagnostic connector](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/diag-conn.jpg)

When pin (10) is shorted to ground (12) ECU enters diagnostic mode.
In this mode pin (1) is used to exchange diagnostic data with a scan tool. Serial communication is done using 1953 baud, 8 bit, 1 stop bit, no parity, TTL(?) levels.
I used the following RS-232 adapter (taken from [Club DSM Colorado](http://co.dsm.org/tech/1G_dl_sch.html)):

![RS-232 adapter 1 schematics](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/ecu-rs232.jpg)

I replaced MAX233 with ST232BD, added required external capacitors and power indication LED:

![RS-232 adapter 2 schematics](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/schematics.gif)
![RS-232 adapter 2 PCB top](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/pcb-top.jpg)
![RS-232 adapter 2 PCB bottom](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/pcb-bottom.jpg)

# About the Protocol
The protocol is simple: request-reply. Request and reply are one-byte message.
I have found requests and reply interpretations on the web one a web page called "DSM Open Source Datalogger Project".
I've found no source there, so I've decided to make one... ;)
There's what I found:

|Sensor|Address|Function|Unit|Notes|
|------|-------|--------|----|-----|
|Accel enrichment|0x1D|100x / 255|%||
|Coolant temperature|0x07|-1.45x + 308|F|Wrong on my E33|
|Engine speed|0x21|31.25x|rpm||
|Fuel trim high|0x0E|0.78x|%||
|Fuel trim middle|0x0D|0.78x|%||
|Fuel trim low|0x0C|0.78x|%||
|Injector pulse width|0x29|0.256x|ms||
|Oxygen feedback trim|0x0F|0.78x|%||
|Oxygen sensor|0x13|0.0195x|V||
|Throttle position|0x17|100x / 255|%||
|Air flow|0x1A|6.29x|Hz||
|Air temperature|0x3A|-1.69x + 358|F|Wrong on my E33|
|Air volume|0x2C|x|||
|Barometer|0x15|0.00486x|bar||
|ISC steps|0x16|x|||
|Knock sum|0x26|x|||
|Timing advance|0x06|x - 10|deg|Wrong on my E33|
|Battery|0x14|0.0733x|V||
|EGR temperature|0x12|-2.7x + 597.7|F|Can't check on E33 M/T|
|TDC|0x02|bit 0x04|||
|Power steering|0x02|bit 0x08|||
|AC Switch|0x02|bit 0x10||Can't check on E33 M/T|
|Park/Neutral|0x02|bit 0x20||Can't check on E33 M/T|
|Idle Switch|0x02|bit 0x80||
|AC Clutch|0x00|bit 0x20||Can't check on E33 M/T|

And here's what I've figured out by trial and error:
|Address|Action|Notes|
|-------|------|-----|
|0xF1|?|0xF1..0xF6 work with engine stopped. After ECU any of theese commands it invokes corresponding relay/solenoid for about 6 seconds. After that it answers with 0x00. If engine is running ECU returns 0xFF immediately.|
|0xF2|?||
|0xF3|?||
|0xF4|?||
|0xF5|Canister purge||
|0xF6|Turn on fuel pump||
|0xF7|Disable injector #6|Can't check on E33 M/T|
|0xF8|Disable injector #5|Can't check on E33 M/T|
|0xF9|Disable injector #4||
|0xFA|Disable injector #3||
|0xFB|Disable injector #2||
|0xFC|Disable injector #1|0xF7..0xFC disable injector for about 6 seconds and return 0x00.|

Diagnostic fault codes are stored as bitmaps in two 8-bit cells.
There're two kinds: active and stored (accumulated).
|Address|Description|
|-------|-----------|
|0x38 (0x4C)|Faults, low byte|
|0x39 (0x4D)|Faults, high byte|
|0x3B (0x4A)|Stored faults, low byte|
|0x3C (0x4B)|Stored faults, high byte|
|0xCA|Clear faults, returns 0x00|

|Bit #|Code|Fault|Notes|
|-----|----|-----|-----|
|1|11|Oxygen sensor||
|2|12|Intake air flow sensor|I haven't checked others|
|3|13|Intake air temperature sensor|I haven't checked others|
|4|14|Throttle position sensor||
|5|15|ISC motor position sensor|I haven't checked others|
|6|21|Engine coolant temperature sensor||
|7|22|Engine speed sensor||
|8|23|TDC sensor||
|9|24|Vehicle speed sensor||
|10|25|Barometric pressure sensor||
|11|31|Knock sensor||
|12|41|Injector circuit||
|13|42|Fuel pump relay||
|14|43|EGR||
|15|44|Ignition coil||
|16|36|Ignition circuit||

# Palm Support
![MMCd](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/mmcd.jpg)

Software is distributed under [GNU GPL](http://www.gnu.org/copyleft/gpl.html). It requires PalmOS 3.5 or higher.
List of supported devices:
|Model|PalmOS|Notes|
|-----|------|-----|
|Palm III|3.0|With OS update|
|Palm IIIc|3.5	
|Palm IIIx|3.1|With OS update|
|Palm IIIxe|3.5	
|Palm V|3.1|With OS update|
|Palm Vx|3.3/3.5|With OS update|
|Palm VIIx|3.5||
|Palm m100|3.5.1||
|Palm m105|3.5.1||
|Palm m125|3.5||
|Palm m130|3.5||
|Palm m500|4.0||
|Palm m505|4.1||
|Palm m515|4.1||
|Palm i705|4.1||
|Palm Tungsten W|4.1.1||
|Sony Clie PEG-N760C|4.1||
|Sony Clie PEG-N710C|3.5.2||
|Sony Clie PEG-N610C|4.01||
|Sony Clie PEG-S360|4.0||
|Sony Clie PEG-S320|4.0||
|Sony Clie PEG-S300|3.5||
|Sony Clie PEG-S500c|3.5||
|Sony Clie PEG-NR70|4.1||
|Sony Clie PEG-NR70V|4.1||
|Sony Clie PEG-T665C|4.1||
|Sony Clie PEG-T615C|4.1||
|Sony Clie PEG-T415|4.1||
|Sony Clie PEG-SL10|4.1||
|Sony Clie PEG-SJ20|4.1||
|Sony Clie PEG-SJ22|4.1||
|Sony Clie PEG-SJ30|4.1||
|Sony Clie PEG-SJ33|4.1||
|Handspring Visor Pro|3.5.2||
|Handspring Visor Platinum|3.5.2||
|Handspring Neo|3.5.2||
|Handspring Visor Prism|3.5.2||
|Handspring Visor Edge|3.5.2||
|Handspring Treo 180|3.5.2||
|Handspring Treo 270|3.5.2||
|Handspring Treo 300|3.5.2||
|Handspring Treo 90|4.1||
|IBM WorkPad c500|4.0||
|IBM WorkPad c505|4.0||
|TRG TRGpro|3.3/3.5.1||
|HandEra|330 3.5.3||

Devices in bold are known to work
Others are supposed to be compatible

List of incompatible devices:
|Model|PalmOS|Notes|
|-----|------|-----|
|Pilot 1000, 5000|v1.0||
|PalmPilot Personal, Professional|v2.0||
|Palm IIIe|v3.1||
|Palm VII|v3.2||
|Palm Zire|4.1|No serial|
|Palm Zire, 71|5.x|No serial/Bad serial|
|Palm Tungsten T|5.0|Bad serial|
|Palm Tungsten C|5.x|Bad serial|
|Handspring Visor|v3.1||
|Handspring Visor Deluxe|v3.1||
|IBM WorkPad original|v3.0||
|IBM WorkPad|v3.1||
|IBM WorkPad c3|v3.1||
|Sony Clie PEG-NX70V|v5.0|Bad serial|
|Sony Clie PEG-NX60|v5.0|Bad serial|
|Sony Clie NZ90|v5.0|Bad serial|
|Sony Clie TG50|v5.0|Bad serial|
|Sony Clie SJ33||Bad serial|
|Qualcomm pdQ 800|v3.02||
|Qualcomm pdQ 1900|v3.02||
|Symbol SPT1500|v3.0.2r3||
|Symbol SPT1700|v3.2||
|Symbol SPT1740|v3.2||

You have to update PalmOS to 3.5+ to use it with:
|Model|PalmOS|Notes|
|-----|------|-----|
|Palm III|v3.0||
|Palm IIIx|v3.1||
|Palm V|v3.1||

# Features
You can continously monitor, graph and record a log of up to 32 parameters (sensors). Logs can be reviewed later.

Main form:

![Main](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/Main.gif)

Tap "New" to start a new log or tap a log name to open an existing one.
You'll see the log form:

![Review](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/Review.gif)

Available parameters (sensors) are divided into four banks of up to eight sensors each. Ar the right top corner is sensor bank selector (1, 2, 3, 4). Below are contents of current bank. Each sensor displayed as sensor name and sensor value.
Tapping at value (the right part) toggles a graph for that sensor. Tapping at sensor name (the left part) toggles sensor capture (more on that later). At the bottom right is mode selector (Review, Monitor). In Review mode recorded log is displayed inside a graph window.
In the middle is a graph window. It can be scrolled using a scrollbar at the bottom or by tapping inside and dragging the cursor. Above graph window is the time when current sample was captured.
Monitor mode looks like this:

![Monitor](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/Monitor.gif)
![Monitor numeric](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/Monitor-numeric.gif)
 
Graph checkbox toggles between graph and numeric mode.
Unchecking "Pause" starts communicaion with the ECU. Selected sensors (ones that have their name highlighted) are polled and graphed.
Checking "Log" turns on log writing. Then it can be reviewed later. At the left bottom there's an indication of sample rate and the sensor being polled. Sample rates dropes when more sensors are enabled.
Test mode:

![Test](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/Test.gif)

Test mode allows read/erase fault codes, activate relays/solenoids and disable injectors. When faults are displayed you can tap code to read what it means.

![Prefs](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/Prefs.gif)
![VSPD](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/VSPD.gif)
 
Preferences dialog lets you define 8 custom ECU locations to log, if you know what they mean. Also there's serial communication speed and measurment system for temperature values (Metric, English and no conversion Numeric).

![Alarms](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/Alarms.gif)

Version 1.3 adds audible alarms support for monitor mode. Sor each sensor you can define alarm threshold, polarity (greater or less than) and choose one of 4 sound pitches.

# Screenshots
Engine start:

![Engine start](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/start.gif)

Knock:

![Knock](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/knock.gif)

Idle on hot engine (ambient temperature is +4C):

![Idle](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/idle1.gif)
![Idle](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/idle2.gif)
![Idle](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/idle3.gif)

Oxygen sensor:

![Oxygen sensor](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/o2.gif)

High-density screen support in v1.4:

![HD screen](https://github.com/illja96/mmcdlogger/blob/main/Dmitry%20Yurtaev/hd.gif)

# Download
[Sourceforge Releases](http://sourceforge.net/project/showfiles.php?group_id=114453)

# Related projects
[MMCd Tools](https://mmcdtools.sourceforge.net): Windows tool to HotSync and export MMCd datalogs to Excel or CSV format.
[Keydiver DSM Chips](https://www.dsmchips.com): Staged performance upgrade chips for 1990-99 DSMs. Also offers a permanent RS232 line into the ECU, eliminating the need for the special datalogging cable.

# Contact Authors
Lead developer: [Dmitry Yurtaev](mailto:yurtaev@users.sourceforge.net)
Additional development: [Shawn Poulson](mailto:spoulson2@users.sourceforge.net), [Clinton Battersby](mailto:cbatters@users.sourceforge.net)
LADA port: [Sergey Kogan](http://sh093.meditprofi.ru)

# Change Log
01-04-2004:
- Shawn Poulson has set up a page for MMCd Tools at [Sourceforge](http://mmcdtools.sourceforge.net) a while ago.
  The MMCd Tools are an open-source set of support applications for handling datalogs.
  It helps to uploads datalogs to PC during HotSync and convert datalogs to CSV or Excel spreadsheets.

04-07-2003: v1.5b
- Added injector duty cycle calculation
- Rearranged sensors (sorry) to make room for INJD

03-07-2003: v1.4c
- If running on a Handspring device call HsKeyboardEnable(false) before trying to open serial port.
Should work on Visors now.

01-07-2003: v1.4b
- Fixed a bug that caused graph scrolling to hang under Visor emulaton.
  It is still feels unresponsive, as event cueue is keeped cluttered by
  penMove events. Wonder if it is the case on real thing.

30-06-2003: v1.4a
- Splitted Edit form into Edit, Test and Debug.
- Moved Edit and Debug modes into they own forms accessible from main menu.
- Added high-density screen support.
  MMCd won't run on Tungstens though, because of limitations of serial port.
- Added double-density icons.
- Fixed a few double-density compatibility isues (cosmetic).
- Added MVIC button to test form.
- Minor conversion corrections for O2 and fuel trims.
- Replaced custom scrollbar with the stock one in Edit form.
- Splitted mmcd.c to test.c and debug.c.
- Moved resource ids definitions from mmcd.h to resource.h.
- Updated help for Preferences dialog.
- Updated Makefile.

20-06-2003: v1.3d
- Another bug in main form: now I was fooled by MemPtrResize.
  If there was more than 16 logs application crashed. Fixed.
- Added 'Backup' attribute indication to main form (dot in the end of log name).
- Added 'Backup on HotSync' checkbox to 'Rename Log' dialog.
- Permanently turned on Debug mode.

20-06-2003: v1.3c
- BUG: Main form code was broken, as it didn't take scroll position into account when calculating which log to open.
  As a result, it was possible to open only 11 first logs.
  Fixed.
- Turned out, that StrCompare() is using some non-obvious method of string compare and there's no plain strcmp() equivalent prior to PalmOS 4.0.
  Rewritten compare() in C.
- Inverted 'S' (P/S load present) bit in f_FLG2().
- Increased precision of conversion for FTxx, O2 voltage.
- In test mode renamed Faults to DTCs (for Diagnostic Trouble Codes).

17-06-2003: v1.3b
- Fixed a small bug with 'Hide' in Review mode: when log contains sample for custom sensor and this sensor is disabled in prefs, HideUnused() shown it anyway leaving a hole in button bank.
- Updated air intake and coolant temperature conversion routines.
  Now with ones found in DSMlink source.
- Added O2-F sensor at 0x3e, renamed O2 to O2-R, deleted AIRV.
- Rearranged sensors.
- Some code cleanup.

16-06-2003: v1.3a
- Added 'Graph' checkbox to Monitor mode.
  When unchecked, graph is replaced by large numeric display of selected sensor.
  Button panel becomes single-selection.
- Added 'Alarms' menu.
  Allows to define alarm threshold and sound for each sensor.
- 'Graph' checkbox is controlled by hardware button 2
- In Monitor/numeric mode hardware button 1 cycles selection thru sensors set for capture.
- In Review mode hardbuttons 1 and 2 move cursor left/right.
- PageUp/PageDown hardware buttons change current bank.
- Hardware button are blocked 1..4 in Edit form whenever unused.
- State of 'H' button, numeric mode and selection is saved on exit.
- Added PalmOS version check (>= 3.5) at startup.
- Removed 'Compatible graphcis' from prefs as it appears unuseful.
- Added 'Alarms' checkbox to prefs.
- Fixed a bug in barometric pressure conversion routine.

11-06-2003: v1.2
- Fixed (I hope) timing advance conversion.
- Added English-mode pressure conversion (in psi).
- in Monitor mode Log and Pause checkboxen can be activated by hardware buttons 3 and 4
  
06-06-2003: v1.1
- Applied patches submitted by Shawn Poulson
- Reduced timeout in EvtGetEvent() as 1 second timeout breaks graph scrolling code.
- Replaced lookup table by XPOSE() macro
- Moved bank selection buttons 1 pixel left off 'H' to make 1-pixel gap between borders as it looks more pleasant to me :)
- Bumped version to 1.1

   Shawn Poulson

   03-06-2003: v1.0d-shawn-2.1
   - Update user interface
   - Update display of sensors in Hide Unused mode, so that they are displayed correctly in column orientation. 
     As put by Dmitry:
     ```
     1 -     1 -   and not... 1 -     1 -
     - -     2 -              - -     2 -
     2 4     3 -              2 4     4 -
     3 -     4 -              3 -     3 -
     ```
   - Update optimization wait in main loop by instead using the commented code block for EvtGetEvent(...).
   This is cleaner and doesn't slow down the GUI by sleeping.

   01-06-2003: v1.0d-shawn-2.0
   - Update user interface
   - Instead of 5th 'X' bank, use 'H' pushbutton in upper right to toggle "Hide Unused" mode.
   - In Review mode, it will dynamically show only sensors logged at the selected graph point.
   - In Monitor mode, it will show only sensors captured or graphed.

   30-05-2003: v1.0d-shawn-1.0
   - Update user inteface
   - Add a 5th 'X' bank that, when selected, will show up to 8 sensors captured or graphed.
   - Add optimization wait in the main loop to keep cpu utilization low.

23-04-2003: v1.0d
- Added "Compatible graphics" checkbox to prefs dialog.
  Looks like direct access to bitmap bits doesn't work on Sony CLIE...

16-04-2003: v1.0c
- Fixed a bug with prefs.
  When running for the first time default preferences get trashed.
  Added "Default" button to prefs dialog.
  (thanks to Steven_GT)
- Removed call to BmpGetBitsDepth unavalable prior to PalmOS 4.
  (thanks to Jeff Oberholtzer)
- try to open serPortCradlePort if serPortCradleRS232Port fails.

31-03-2003: v1.0b
- Implemented baud rate and measure system in prefs dialog
- Fixed Purge command in Test form. Pressure, EGR and Boost need testing.

28-03-2003: 
- Added scrollbar to main form (it still needs some work, though)
- Implemented test mode.
  Injectors, Fuel pump, Fault erase should work as expected.
  Fault codes are not checked throughly - some assistance needed.
  Others may work, may not, may be permutated
- Removed Debug mode
- Added prefs dialog

27-01-2003: v1.0a
- Main form doesn't scroll, so up to 11 logs can be selected
- Test mode not implemented yet and contains debugging stuff
- Settings dialog not implemented
