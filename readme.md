This project is about controlling clocks.
Part of project written in kotlin just in order to create prototype for interface.
Another part is actual Arduino project making all process.

Serial interface API.

***
General request format:
{cmd}#{id}#{body}\n

cmd - String. Command name. Options: [info/mode/color/setup/ok/cancel/time].
id - Integer. Id command for netting respone. This value is forwarding to response. In case of value = "-1" reply is not needed.
body - String. Cmd body.
Cmd parts separator - #.
\n - cmd end (carrige return).
***

---------------------------

***
General response format:
^#{cmd}#{direction}#{id}#{body}#$

CMD begin marker - ^
cmd - String. Command name. Options: [info/mode/color/setup/ok/cancel/time].
direction - String. Direction. Optons: [req/rsp].
id - Integer. Id command for netting respone with request. If direction == request value == -1.
body - String. Cmd body.
cmd parts separator - #.
CMD end marker - $
***

---------------------------

***
info
*request*

cmd - "info"
id - "1"
body - empty

Example:
info#1#

*respone*
^#info#rsp#1#time: 15:22:56; timeMode: withSeconds; possible time modes: [withSeconds, noSeconds, secondsOnDetect]; is global setup mode: 0; colorScheme: redDragon; possible color schemes: [blueLagoon, redDragon, fadeToGray, greenForrest]#$
***

----------------------------

***
mode
*request*

cmd - "mode"
id - "2"
body - one of possible values: [0 = withSeconds, 1 = noSeconds, 2 = secondsOnDetect]

Example:
mode#2#withSeconds

*respone*
^#mode#rsp#2#mode switched to: 2#$
***

----------------------------

***
color
*request*

cmd - "color"
id - "3"
body - one of possible values: [0 = blueLagoon, 1 = redDragon, 2 = fadeToGray, 3 = greenForrest]

Example:
color#3#redDragon

*respone*
^#color#rsp#3#color switched to: 0#$
***

----------------------------

***
setup
*request*

cmd - "setup"
id - "4"
body - empty

Example:
setup#4#

*respone*
^#setup#rsp#4#Global setup is on.#$
***

----------------------------

***
ok
*request*

cmd - "ok"
id - "5"
body - empty	

Example:
ok#5#

*respone*
if global setup mode: 1 -> ^#ok#rsp#5#New time is set. Global setup is off#$
if global setup mode: 0 -> ^#ok#rsp#5#cmd ok is ignored because global setup mode if off.#$
***

----------------------------

***
cancel
*request*

cmd - "cancel"
id - "6"
body - empty	

Example:
cancel#6#

*respone*
if global setup mode: 1 -> ^#cancel#rsp#6#Returned to previous time. Global setup is off#$
if global setup mode: 0 -> ^#cancel#rsp#6#cmd cancel is ignored because global setup mode if off.#$
***

----------------------------

***
time
*request*

cmd - "time"
id - "7"
body - "hh:mm:ss"	

Example:
time#7#16:21:43

*respone*
if global setup mode: 1 -> ^#time#rsp#7#Presetting new time.#$
if global setup mode: 0 -> ^#time#rsp#7#time is ignored because global setup mode if off.#$
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
^#onModeChanged#req#-1#current mode: 0#$

*respone*
no
***

----------------------------


Examples:
^#info#rsp#3#time: 23:20:31; timeMode: 1; possible time modes: [withSeconds, noSeconds, secondsOnDetect]; is global setup mode: 0; colorScheme: 3; possible color schemes: [blueLagoon, redDragon, fadeToGray, greenForrest]#$

^#mode#rsp#3#mode switched to: 0#$

^#mode#rsp#3#mode switched to: 1#$

^#mode#rsp#3#mode switched to: 2#$

^#color#rsp#3#color switched to: 1#$

^#ok#rsp#3#cmd ok is ignored because global setup mode if off.#$

^#cancel#rsp#3#cancel ok is ignored because global setup mode if off.#$

^#time#rsp#3#time is ignored because global setup mode if off.#$

^#time#rsp#3#Presetting new time.#$
