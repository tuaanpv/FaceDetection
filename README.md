This is the source code that I created in 2016 to learn about Computer Vision (OpenCV C++). It detects and recognizes human faces and then sends notifications to a phone number via SMS.
Demo on Laptop and Orange Pi: https://youtu.be/XC400-zM8RM?si=YR5AfJ0he3BLl68B

/* SIM808 get date time
AT+COPS=2
AT+COPS=0
result:
----------------------------------------
AT+COPS=2
OK
AT+COPS=0
*PSUTTZ: 2017,3,31,16,52,4,"+28",0

DST: 0

OK
-------------------------------------------

Then: 
AT+CCLK?
--------------------------------------------

GSM_Send_Modem_Command("AT+COPS=2\r\n");//de register
GSM_Send_Modem_Command("AT+CTZU=1\r\n");//automatic time zone update is enabled
GSM_Send_Modem_Command("AT+COPS=0\r\n");// register to network
GSM_Send_Modem_Command("AT+CCLK?\r\n");// gets the clock of the MT
*/

/*
Terminal Set date time linux
Use date -s: sudo date -s "2 OCT 2006 18:00:00"

date -s '2014-12-25 12:34:56'
Run that as root or under sudo. Changing only one of the year/month/day is more of a challenge and will involve repeating bits of the current date. There are also GUI date tools built in to the major desktop environments, usually accessed through the clock.

To change only part of the time, you can use command substitution in the date string:

date -s "2014-12-25 $(date +%H:%M:%S)"
will change the date, but keep the time. See man date for formatting details to construct other combinations: the individual components are %Y, %m, %d, %H, %M, and %S.
*/
