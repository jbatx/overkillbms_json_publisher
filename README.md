## OverkillSolar BMS data retriever and json poster
In simplest terms, this repo is meant to facilitate using an esp32, pull data from an Overkill Solar bms, format it as json and post it to a URL.  This repo
includes serverless code that uses aws for receiving the post and storing it.  Since an Overkill bms is a tested and support JBD bms, it'll with
those too.  I built this around a 16s 48v Overkill Solar bms.
   
Here's the related project post https://diysolarforum.com/threads/anyone-working-with-the-overkill-solar-arduino-lib.27811/page-3

#####As of 9/24/21 This is incomplete and patially working.  These are the issues

1. This sketch depends on https://github.com/FurTrader/Overkill-Solar-BMS-Arduino-Library for serial communication with the bms.  
However, to get it it to compile with the esp32 Arduino lib it needed to be cleaned up and fixed.   
The bms.cpp version in this repo contains those fixes.  Until I get permission to branch and PR on the original repo, 
I'm leaving this copy right here.

2. overkill_

## Server ..less side
You can, of course, post to whatever URL you like.  I won't go into details about the options because
 I assume that, if you're reading this, you probably have some idea of where you want your json 
 iot payloads to go.  If that's a foreign language... well, hit the books, as they say :-)
 
 If you are sufficiently good at the Interwebz and cloud things, keep reading.
 
#### Serverless - AWS API Gateway to Lambda to Postgres db
This setup was *based* on this aws how-to https://docs.aws.amazon.com/lambda/latest/dg/services-rds-tutorial.html.
I used a different db name and table.  I also chose Postgres over MySQL ...because professional preference reasons.

####Setup and Runtime Costs
This option will cost you about $14 per month to run on a db.t4g.micro rds instance + lambda and api 
gateway costs and whatever else you add-on.  I chose this route because it'll work reliably with minimal set up time.


##Developer notes
####Python environment
`cd aws`

`source iot/bin/activate`


