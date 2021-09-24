# overkillbms_json_poster
Using an esp32, pull date form an overkill bms, format it as json and post it to a URL.   Here's the related project post https://diysolarforum.com/threads/anyone-working-with-the-overkill-solar-arduino-lib.27811/page-3

As of 9/24/21 This is incomplete and not yet working.  These are the issues
# bms.cpp file is not tied to original authors repo.
1. This sketch depends on https://github.com/FurTrader/Overkill-Solar-BMS-Arduino-Library for serial communication with the bms.  However, to get it it to compile with the esp32 Arduino lib it needed to be cleaned up and fixed.   The bms.cpp version in this repo contains those fixes.  Until I get permission to branch and PR on the original repo, I'm leaving this copy right here.
2. I believe the esp32 and bms need a line logic converter before communication can happen.  This is because the esp32 UART is 3.3v and the bms UART is 5v.  Once I have that tested and working, I'll update this repo.
