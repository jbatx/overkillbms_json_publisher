## OverkillSolar BMS data retriever and json poster
In simplest terms, this repo is meant to facilitate using an esp32, pull data from an Overkill Solar bms, format it as json and post it to a URL OR Publish to an MQTT topic on AWS IoT Core.  The latter is the default and you'll have to change things up in the code to use the url post function.  Should be pretty easy.

This repo includes serverless code that uses aws for receiving the post and storing it.  Since an Overkill bms is a tested and support JBD bms, it'll with
those too.  I built this around a 16s 48v Overkill Solar bms.
   
Here's the related project post https://diysolarforum.com/threads/anyone-working-with-the-overkill-solar-arduino-lib.27811/page-3

Watch this video to see how to get and configure the AWS_IOT (MQTT) lib used in the sketch OR if you need help setting up AWS IoT Core: https://www.youtube.com/watch?v=2y0w977q_yk

#####As of 9/24/21 This is incomplete and patially working.  These are the issues

1. This sketch depends on https://github.com/FurTrader/Overkill-Solar-BMS-Arduino-Library for serial communication with the bms.  
However, to get it it to compile with the esp32 Arduino lib it needed to be cleaned up and fixed.   
The bms.cpp version in this repo contains those fixes.  Until I get permission to branch and PR on the original repo, 
I'm leaving this copy right here. 

####Regarding the above

You must copy or symlink the Overkill-Solar-BMS-Arduino-Library lib in this repo to your Arduino/libraries directory


## Circuit parts and wiring

####What I used

1. KeeYees WROOM ESP32 which is apparently the same as a NodeMCU ESP32S https://www.amazon.com/dp/B07QCP2451?psc=1&ref=ppx_yo2_dt_b_product_details
2. KeeYees logic level converter https://www.amazon.com/dp/B07LG646VS?psc=1&ref=ppx_yo2_dt_b_product_details
3. Small breadboards...
4. Jumper wires...

I don't plan to make a wiring diagram because there are many youtube vids and articles on wiring a logic converter to an esp.

####Mistakes I made
1. Forgetting to connect the ground from the bms to the esp32
2. reversing the rx and tx params to the begin() method
3. not reversing the rx and tx pins connecting the esp32 to the bms
4. not using (or knowing) what a logic converter is
5. debugging this at 2am
6. conecting the 5v power to the low voltage side of the logic converter

# Serverless - AWS IoT Core

####Setup and Runtime Costs
Based on the calculator that AWS provides, this message size and polling interval will cost less than $.2 (twenty cents) per month per bms.

The basic AWS IoT Core set up steps are
1. Create a Thing in the console
2. Create a "classic" shadow
3. Create a certificate and download them along with the Amazon CA1 cert
4. Create a policy
5. Make sure the policy, cert and thing are all attached to one another
6. Update the aws_iot_certificates.c file (watch the youtube video linked above)
 
##Developer notes
The bms::get_cell_voltage() has been modified to return millivolts instead of volts because I want the raw data.  Also, it was roundig it to the nearest 100th.

