This project is about controlling clocks.
Part of project written in kotlin just in order to create prototype for interface.
Another part is actual Arduino project making all process.

Serial interface API.

***
General request format:
{cmd}|{id}|{body}\n

cmd - String. Command name. Options: [info/mode/color/setup/ok/cancel/time].
id - Integer. Id command for netting respone. This value is forwarding to response. In case of value = "-1" reply is not needed.
body - String. Cmd body.
| - cmd parts separator.
\n - cmd end (carrige return).
***

---------------------------

***
General response format:
BEG|{cmd}|{direction}|{id}|{body}|END

BEG - CMD begin marker.
cmd - String. Command name. Options: [info/mode/color/setup/ok/cancel/time].
direction - String. Direction. Optons: [req/rsp].
id - Integer. Id command for netting respone with request. If direction == request value == -1.
body - String. Cmd body.
| - cmd parts separator.
END - CMD end marker.
***

---------------------------

***
info
*request*

cmd - "info"
id - "1"
body - empty

Example:
info|1|

*respone*
BEG|info|rsp|1|time: 15:22:56; timeMode: withSeconds; possible time modes: [withSeconds, noSeconds, secondsOnDetect]; is global setup mode: 0; colorScheme: redDragon; possible color schemes: [blueLagoon, redDragon, fadeToGray, greenForrest]|END
***

----------------------------

***
mode
*request*

cmd - "mode"
id - "2"
body - one of possible values: [0 = withSeconds, 1 = noSeconds, 2 = secondsOnDetect]

Example:
mode|2|withSeconds

*respone*
BEG|mode|rsp|2|mode switched to: 2|END
***

----------------------------

***
color
*request*

cmd - "color"
id - "3"
body - one of possible values: [0 = blueLagoon, 1 = redDragon, 2 = fadeToGray, 3 = greenForrest]

Example:
color|3|redDragon

*respone*
BEG|color|rsp|3|color switched to: 0|END
***

----------------------------

***
setup
*request*

cmd - "setup"
id - "4"
body - empty

Example:
setup|4|

*respone*
BEG|setup|rsp|4|Global setup is on.|END
***

----------------------------

***
ok
*request*

cmd - "ok"
id - "5"
body - empty	

Example:
ok|5|

*respone*
if global setup mode: 1 -> BEG|ok|rsp|5|New time is set. Global setup is off|END
if global setup mode: 0 -> BEG|ok|rsp|5|cmd ok is ignored because global setup mode if off.|END
***

----------------------------

***
cancel
*request*

cmd - "cancel"
id - "6"
body - empty	

Example:
cancel|6|

*respone*
if global setup mode: 1 -> BEG|cancel|rsp|6|Returned to previous time. Global setup is off|END
if global setup mode: 0 -> BEG|cancel|rsp|6|cmd cancel is ignored because global setup mode if off.|END
***

----------------------------

***
time
*request*

cmd - "time"
id - "7"
body - "hh:mm:ss"	

Example:
time|7|16:21:43

*respone*
if global setup mode: 1 -> BEG|time|rsp|7|Presetting new time.|END
if global setup mode: 0 -> BEG|time|rsp|7|time is ignored because global setup mode if off.|END
***

----------------------------


REQUESTS INITIATING.
***
onModeChanged
*request*

cmd - "onModeChanged"
id - "-1"
body - "current mode: 0"	

Example:
BEG|onModeChanged|req|-1|current mode: 0|END

*respone*
no
***

----------------------------