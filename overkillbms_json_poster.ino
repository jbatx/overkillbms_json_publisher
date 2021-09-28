#include "bms.h"
#include "options.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "AWS_IOT.h"

#define CLIENT_ID "test_3"// thing unique ID, this id should be unique among all things associated with your AWS account.
#define MQTT_TOPIC "$aws/things/test3/shadow/update" //topic for the MQTT data
#define AWS_HOST "a1a2z1xtkva2kp-ats.iot.us-east-2.amazonaws.com" // your host for uploading data to AWS,

AWS_IOT aws;

const char* host = "https://postb.in/1631759911516-7197994671296";

OverkillSolarBms bms = OverkillSolarBms();
uint32_t last_soc_check_time;

#define SOC_POLL_RATE 2000  // milliseconds

HardwareSerial HWSerial(2); // Define a Serial port instance called 'Receiver' using serial port 2

#define Receiver_Txd_pin 17
#define Receiver_Rxd_pin 16

float voltage;
float current;
uint8_t soc;
float lowest;
float highest;
float voltageDiff;
bool cFet;
bool dFet;
bool single_cell_overvoltage_protection; 
bool single_cell_undervoltage_protection; 
bool whole_pack_overvoltage_protection; 
bool whole_pack_undervoltage_protection; 
bool charging_over_temperature_protection; 
bool charging_low_temperature_protection; 
bool discharge_over_temperature_protection; 
bool discharge_low_temperature_protection; 
bool charging_overcurrent_protection; 
bool discharge_overcurrent_protection; 
bool short_circuit_protection; 
bool front_end_detection_ic_error;
bool software_lock_mos;



void setup() {
    static const uint8_t LED_BUILTIN = 2;
    // Connect to BMS
    pinMode(LED_BUILTIN, OUTPUT);
    //Serial1.begin(9600);

    Serial.begin(115200);                                                   // Define and start serial monitor
    HWSerial.begin(9600, SERIAL_8N1, Receiver_Rxd_pin, Receiver_Txd_pin);
    
    bms.begin(&HWSerial);
    bms.set_query_rate(2000);  // Set query rate to 2000 milliseconds (2 seconds)
    last_soc_check_time = 0;

    // Connect to wifi
    const char* ssid     = "Satau";
    const char* password = "losthorizontx";
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Connect to AWS MQTT
    if(aws.connect(AWS_HOST, CLIENT_ID) == 0){ // connects to host and returns 0 upon success
      Serial.println("  Connected to AWS\n  Done.");
    }else {
      Serial.println("  Connection failed!\n make sure your subscription to MQTT in the test page");
    }


}

void loop() {
    bms.main_task();
    //bms.get_comm_error_state();

    //bms.debug();
    uint8_t soc = bms.get_state_of_charge();

    if (millis() - last_soc_check_time > SOC_POLL_RATE) {

         String jsonStr = "{";
         /**
         Gether up data from the bms
         */

         // Get voltage
        float tmpVoltage = bms.get_voltage();
        if (tmpVoltage != voltage) {
          jsonStr += "\"voltage\":" + (String)voltage;
        }
        
         // Get current
        float tmpCurrent = bms.get_current();
        if (tmpCurrent != current) {
          current = tmpCurrent;
          jsonStr += ",\"current\":" + (String)current;
        }     

        // Get voltage diff
        float tmpHighest;
        tmpHighest = 99;
        float tmpLowest;
        tmpLowest = 0;
        float v = 0;
        for(int i=0; i < 16; i++){
          v = bms.get_cell_voltage(i);
          if(v < lowest){
            tmpLowest = v;
          }
          if(v > highest){
            tmpHighest = v;
          }
        }

        if (tmpLowest != lowest) {
          lowest = tmpLowest;
          jsonStr += ",\"lowestCell\":" + (String)lowest;
        }

        if (tmpHighest != highest) {
          highest = tmpHighest;
          jsonStr += ",\"highestCell\":" + (String)highest;
        }
        if (tmpHighest - tmpLowest != voltageDiff) {
          voltageDiff = highest - lowest;
          jsonStr += ",\"voltageDiff\":" + (String)voltageDiff;
        }
        
        // Get state of charge
        uint8_t tmpSoc = bms.get_state_of_charge();
        if(tmpSoc != soc){
          soc = tmpSoc;
          jsonStr += ",\"soc\":" + (String)soc;
        }
        
        // Get charge fet status
        uint8_t tmpCFet = bms.get_charge_mosfet_status() ? 1 : 0;
        if (tmpCFet != cFet) {
          cFet = tmpCFet;
          jsonStr += ",\"cFet\":" + (String)cFet;
        }

        // Get discharge fet status
        uint8_t tmpDFet = bms.get_discharge_mosfet_status() ? 1 : 0;
        if (tmpDFet != cFet) {
          dFet = tmpDFet;
          jsonStr += ",\"dFet\":" + (String)dFet;
        }
      
        ProtectionStatus flags = bms.get_protection_status();

        bool tmp_single_cell_overvoltage_protection = flags.single_cell_overvoltage_protection;
        if ( tmp_single_cell_overvoltage_protection != single_cell_overvoltage_protection) {
          single_cell_overvoltage_protection = tmp_single_cell_overvoltage_protection;
          jsonStr += "\"single_cell_overvoltage_protection\":" + (String)flags.single_cell_overvoltage_protection;
        }

        bool tmp_single_cell_undervoltage_protection = flags.single_cell_undervoltage_protection; 
        if ( tmp_single_cell_undervoltage_protection != single_cell_undervoltage_protection) {
          single_cell_undervoltage_protection = tmp_single_cell_undervoltage_protection;
          jsonStr += ",\"single_cell_undervoltage_protection\":" + (String)flags.single_cell_undervoltage_protection;
        }
        bool tmp_whole_pack_overvoltage_protection = flags.whole_pack_overvoltage_protection;
        if ( tmp_whole_pack_overvoltage_protection != whole_pack_overvoltage_protection) {
          whole_pack_overvoltage_protection = tmp_whole_pack_overvoltage_protection;
          jsonStr += ",\"whole_pack_overvoltage_protection\":" + (String)flags.whole_pack_overvoltage_protection;
        }
        bool tmp_whole_pack_undervoltage_protection = flags.whole_pack_undervoltage_protection; 
        if ( tmp_whole_pack_undervoltage_protection != whole_pack_undervoltage_protection) {
          whole_pack_undervoltage_protection = tmp_whole_pack_undervoltage_protection;
          jsonStr += ",\"whole_pack_undervoltage_protection\":" + (String)flags.whole_pack_undervoltage_protection;
        }
        bool tmp_charging_over_temperature_protection = flags.charging_over_temperature_protection; 
        if ( tmp_charging_over_temperature_protection != charging_over_temperature_protection) {
          charging_over_temperature_protection = tmp_charging_over_temperature_protection;
          jsonStr += ",\"charging_over_temperature_protection\":" + (String)flags.charging_over_temperature_protection;
        }
        bool tmp_charging_low_temperature_protection = flags.charging_low_temperature_protection;
        if ( tmp_charging_low_temperature_protection != charging_low_temperature_protection) {
          charging_low_temperature_protection = tmp_charging_low_temperature_protection;
          jsonStr += ",\"charging_low_temperature_protection\":" + (String)flags.charging_low_temperature_protection;
        } 
        bool tmp_discharge_over_temperature_protection = flags.discharge_over_temperature_protection; 
        if ( tmp_discharge_over_temperature_protection != discharge_over_temperature_protection) {    
          discharge_over_temperature_protection = tmp_discharge_over_temperature_protection;
          jsonStr += ",\"discharge_over_temperature_protection\":" + (String)flags.discharge_over_temperature_protection;
        }
        bool tmp_discharge_low_temperature_protection = flags.discharge_low_temperature_protection;
        if ( tmp_discharge_low_temperature_protection != discharge_low_temperature_protection) {
          discharge_low_temperature_protection = tmp_discharge_low_temperature_protection;
          jsonStr += ",\"discharge_low_temperature_protection\":" + (String)flags.discharge_low_temperature_protection;
        } 
        bool tmp_charging_overcurrent_protection = flags.charging_overcurrent_protection;
        if ( tmp_charging_overcurrent_protection != charging_overcurrent_protection) {
          charging_overcurrent_protection = tmp_charging_overcurrent_protection;
          jsonStr += ",\"charging_overcurrent_protection\":" + (String)flags.charging_overcurrent_protection;
        }
        bool tmp_discharge_overcurrent_protection = flags.discharge_overcurrent_protection;
        if ( tmp_discharge_overcurrent_protection != discharge_overcurrent_protection) {
          discharge_overcurrent_protection = tmp_discharge_overcurrent_protection;
          jsonStr += ",\"discharge_overcurrent_protection\":" + (String)flags.discharge_overcurrent_protection;
        } 
        bool tmp_short_circuit_protection = flags.short_circuit_protection;
        if ( tmp_short_circuit_protection != short_circuit_protection) {
          short_circuit_protection = tmp_short_circuit_protection;
          jsonStr += ",\"short_circuit_protection\":" + (String)flags.short_circuit_protection;
        } 
        bool tmp_front_end_detection_ic_error = flags.front_end_detection_ic_error;
        if ( tmp_front_end_detection_ic_error != front_end_detection_ic_error) {
          front_end_detection_ic_error = tmp_front_end_detection_ic_error;
          jsonStr += ",\"front_end_detection_ic_error\":" + (String)flags.front_end_detection_ic_error;
        }
        bool tmp_software_lock_mos = flags.software_lock_mos;
        if ( tmp_software_lock_mos != software_lock_mos) {
          software_lock_mos = tmp_software_lock_mos;
          jsonStr += ",\"software_lock_mos\":" + (String)flags.software_lock_mos;
        }

        // Get bms name
        String name = bms.get_bms_name();

        // Build the json we'll send
        if ( jsonStr.length() > 1) {
          jsonStr += ",\"name\":\"" + (String)"\"" + name + (String)"\"";
          jsonStr += "}";
          Serial.print(jsonStr);
          Serial.println();
          publishMqtt((String)jsonStr);
        } else {
          Serial.println("Nothing has chagned");
        }
      
         // record the time before we hit the server
         last_soc_check_time = millis();

         /**
          Send the data through the interw3bz
          */
         //postData();
         
    }
}

void publishMqtt(String msg){
  //create string payload for publishing
  int len = msg.length();
  char payload[len];
  msg.toCharArray(payload, len);

  Serial.println("Publishing:- ");
  Serial.println(payload);
  if(aws.publish(MQTT_TOPIC, payload) == 0){// publishes payload and returns 0 upon success
    Serial.println("Success\n");
  }else{
    Serial.println("Failed!\n");
  }
}

void postData(){
  // Use HTTPClient to connect
    const char* url = "https://postb.in/1631765636111-0645570172928";
    Serial.print("Requesting URL: ");
    Serial.println(url);
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpRequestData = "foo=123&bar=baz";    
    long rn;       
    rn = random(200);
    httpRequestData += "&randnum=";
    httpRequestData += rn;
    int httpCode = http.POST(httpRequestData);
        
    if(httpCode > 0){  
      String payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload);
    } else {
      Serial.println("Error on HTTP request");
    }

    http.end();
}
