#include "bms.h"
#include "options.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "AWS_IOT.h"

#define CLIENT_ID "battery_1"// thing unique ID, this id should be unique among all things associated with your AWS account.
#define MQTT_TOPIC "$aws/things/test3/shadow/update" //topic for the MQTT data
#define AWS_HOST "123123" // your host for uploading data to AWS,

AWS_IOT aws;

const char* host = "https://postb.in/1631759911516-7197994671296";

OverkillSolarBms bms = OverkillSolarBms();
uint32_t last_soc_check_time;

#define SOC_POLL_RATE 300000  // send evert five minutes (milliseconds)

HardwareSerial HWSerial(2); // Define a Serial port instance called 'Receiver' using serial port 2

#define Receiver_Txd_pin 17
#define Receiver_Rxd_pin 16

float voltage;
float current;
float soc;
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
std::vector<float> cellVoltages(16);
std::vector<float> cellBalanceStatus(16);
std::vector<float> tempSensors(3);

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

    if (millis() - last_soc_check_time > SOC_POLL_RATE ||  last_soc_check_time == 0) {

         String jsonStr = "{";
         /**
         Gether up data from the bms
         */

         // Get voltage
        float tmpVoltage = bms.get_voltage();
        if (tmpVoltage != voltage) {
          voltage = tmpVoltage;
          jsonStr += "\"voltage\":" + (String)voltage + setDelim();
        }
        
         // Get current
        float tmpCurrent = bms.get_current();
        Serial.print("current: ");
        Serial.println(tmpCurrent);
        
        if (tmpCurrent != current) {
          current = tmpCurrent;
          jsonStr += "\"current\":" + (String)current + setDelim();
        }  

        // Get state of charge
        float tmpSoc = bms.get_state_of_charge();
        Serial.print("SOC: ");
        Serial.print(tmpSoc);
        Serial.print(" ");
        Serial.println(soc);
        if(tmpSoc != soc){
          soc = tmpSoc;
          jsonStr += "\"soc\":" + (String)soc + setDelim();
        }
        
        // Get charge fet status
        uint8_t tmpCFet = bms.get_charge_mosfet_status() ? 1 : 0;
        if (tmpCFet != cFet) {
          cFet = tmpCFet;
          jsonStr += "\"c_fet\":" + (String)cFet + setDelim();
        }

        // Get discharge fet status
        uint8_t tmpDFet = bms.get_discharge_mosfet_status() ? 1 : 0;
        if (tmpDFet != dFet) {
          dFet = tmpDFet;
          jsonStr += "\"d_fet\":" + (String)dFet + setDelim();
        }   

        // Get cell balancing status
        bool bs;
        int cNum = 0;
        for(int i=0; i < 16; i++){
          
          bs = bms.get_balance_status(i);
          cNum = i + 1; // for easily interpretable names
          Serial.print("Cell ");
          Serial.print(cNum);
          Serial.print(" Balance status:");
          Serial.print(bs);
          Serial.print(" previous was ");
          Serial.println(cellBalanceStatus[i]);

          // See if balance status has changed
          if (cellBalanceStatus[i] != bs){
            cellBalanceStatus[i] = bs;
            jsonStr += "\"c_" + (String)cNum + "_balancing\":" + (String)bs + setDelim();
          }
        }

        // Get temp sensor vals
        float t;
        int tNum = 0;
        for (uint8_t i=0; i<bms.get_num_ntcs(); i++) {
          
          t = bms.get_ntc_temperature(i);
          tNum = i + 1; // for easily interpretable names
          Serial.print("temp sensor ");
          Serial.print(tNum);
          Serial.print(" temp:");
          Serial.print(t);
          Serial.print(" previous was ");
          Serial.println(tempSensors[i]);

          // See if temp has changed
          if (tempSensors[i] != t){
            tempSensors[i] = t;
            jsonStr += "\"ts_" + (String)tNum + "\":" + (String)t + setDelim();
          }
        }


        // Get voltage diff and cell voltages
        float tmpHighest = 0;
        float tmpLowest = 9999;
        float v = 0;
        cNum = 0;
        for(int i=0; i < 16; i++){

          v = bms.get_cell_voltage(i);
          cNum = i + 1; // for easily interpretable names
    
          Serial.print(cNum);
          Serial.print("Cell voltage: ");
          Serial.print(v);
          Serial.print(" Previous was ");
          Serial.println(cellVoltages[i]);

          // See if cell voltage has changed
          if (cellVoltages[i] != v){
            cellVoltages[i] = v;
            jsonStr += "\"c_" + (String)cNum + "_v\":" + (String)v + setDelim();
          }
          if(v < tmpLowest){
            tmpLowest = v;
          }
          if(v > tmpHighest){
            tmpHighest = v;
          }
        }

        if (tmpLowest != lowest) {
          lowest = tmpLowest;
          jsonStr += "\"lcv\":" + (String)lowest + setDelim();
        }

        if (tmpHighest != highest) {
          highest = tmpHighest;
          jsonStr += "\"hcv\":" + (String)highest + setDelim();
        }
        float tmpVoltageDiff = tmpHighest - tmpLowest;
        if ( tmpVoltageDiff != voltageDiff) {
          voltageDiff = highest - lowest;
          jsonStr += "\"v_diff\":" + (String)voltageDiff + setDelim();
        }
      
        ProtectionStatus flags = bms.get_protection_status();

        bool tmp_single_cell_overvoltage_protection = flags.single_cell_overvoltage_protection;
        if ( tmp_single_cell_overvoltage_protection != single_cell_overvoltage_protection) {
          single_cell_overvoltage_protection = tmp_single_cell_overvoltage_protection;
          jsonStr += "\"sc_overvoltage_p\":" + (String)flags.single_cell_overvoltage_protection + setDelim();
        }

        bool tmp_single_cell_undervoltage_protection = flags.single_cell_undervoltage_protection; 
        if ( tmp_single_cell_undervoltage_protection != single_cell_undervoltage_protection) {
          single_cell_undervoltage_protection = tmp_single_cell_undervoltage_protection;
          jsonStr += "\"sc_undervoltage_p\":" + (String)flags.single_cell_undervoltage_protection + setDelim();
        }
        bool tmp_whole_pack_overvoltage_protection = flags.whole_pack_overvoltage_protection;
        if ( tmp_whole_pack_overvoltage_protection != whole_pack_overvoltage_protection) {
          whole_pack_overvoltage_protection = tmp_whole_pack_overvoltage_protection;
          jsonStr += "\"wp_overvoltage_p\":" + (String)flags.whole_pack_overvoltage_protection + setDelim();
        }
        bool tmp_whole_pack_undervoltage_protection = flags.whole_pack_undervoltage_protection; 
        if ( tmp_whole_pack_undervoltage_protection != whole_pack_undervoltage_protection) {
          whole_pack_undervoltage_protection = tmp_whole_pack_undervoltage_protection;
          jsonStr += "\"wp_undervoltage_p\":" + (String)flags.whole_pack_undervoltage_protection + setDelim();
        }
        bool tmp_charging_over_temperature_protection = flags.charging_over_temperature_protection; 
        if ( tmp_charging_over_temperature_protection != charging_over_temperature_protection) {
          charging_over_temperature_protection = tmp_charging_over_temperature_protection;
          jsonStr += "\"ch_over_temp_p\":" + (String)flags.charging_over_temperature_protection + setDelim();
        }
        bool tmp_charging_low_temperature_protection = flags.charging_low_temperature_protection;
        if ( tmp_charging_low_temperature_protection != charging_low_temperature_protection) {
          charging_low_temperature_protection = tmp_charging_low_temperature_protection;
          jsonStr += "\"ch_low_temp_p\":" + (String)flags.charging_low_temperature_protection + setDelim();
        } 
        bool tmp_discharge_over_temperature_protection = flags.discharge_over_temperature_protection; 
        if ( tmp_discharge_over_temperature_protection != discharge_over_temperature_protection) {    
          discharge_over_temperature_protection = tmp_discharge_over_temperature_protection;
          jsonStr += "\"disch_over_temp_p\":" + (String)flags.discharge_over_temperature_protection + setDelim();
        }
        bool tmp_discharge_low_temperature_protection = flags.discharge_low_temperature_protection;
        if ( tmp_discharge_low_temperature_protection != discharge_low_temperature_protection) {
          discharge_low_temperature_protection = tmp_discharge_low_temperature_protection;
          jsonStr += "\"disch_low_temp_p\":" + (String)flags.discharge_low_temperature_protection + setDelim();
        } 
        bool tmp_charging_overcurrent_protection = flags.charging_overcurrent_protection;
        if ( tmp_charging_overcurrent_protection != charging_overcurrent_protection) {
          charging_overcurrent_protection = tmp_charging_overcurrent_protection;
          jsonStr += "\"ch_overcurrent_p\":" + (String)flags.charging_overcurrent_protection + setDelim();
        }
        bool tmp_discharge_overcurrent_protection = flags.discharge_overcurrent_protection;
        if ( tmp_discharge_overcurrent_protection != discharge_overcurrent_protection) {
          discharge_overcurrent_protection = tmp_discharge_overcurrent_protection;
          jsonStr += "\"disch_overcurrent_p\":" + (String)flags.discharge_overcurrent_protection + setDelim();
        } 
        bool tmp_short_circuit_protection = flags.short_circuit_protection;
        if ( tmp_short_circuit_protection != short_circuit_protection) {
          short_circuit_protection = tmp_short_circuit_protection;
          jsonStr += "\"short_circuit_p\":" + (String)flags.short_circuit_protection + setDelim();
        } 
        bool tmp_front_end_detection_ic_error = flags.front_end_detection_ic_error;
        if ( tmp_front_end_detection_ic_error != front_end_detection_ic_error) {
          front_end_detection_ic_error = tmp_front_end_detection_ic_error;
          jsonStr += "\"front_end_detection_ic_error\":" + (String)flags.front_end_detection_ic_error + setDelim();
        }
        bool tmp_software_lock_mos = flags.software_lock_mos;
        if ( tmp_software_lock_mos != software_lock_mos) {
          software_lock_mos = tmp_software_lock_mos;
          jsonStr += "\"sw_lock_mos\":" + (String)flags.software_lock_mos + setDelim();
        }

        // Build the json we'll send
        if ( jsonStr.length() > 1) {
        
          // Add the device_id
          jsonStr += "\"device_id\":\"" + (String)CLIENT_ID + "\"" + setDelim();

          // add bms model
          String name = bms.get_bms_name();
          jsonStr += "\"model\":\"" + name + "\"" + setDelim();
          jsonStr += "}\n";

          Serial.println();
          publishMqtt(jsonStr);
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
  if (aws.publish(MQTT_TOPIC, payload) == 0){// publishes payload and returns 0 upon success
    Serial.println("Success\n");
  }else{
    Serial.println("Failed! waiting and then will try again....\n");
    delay(1000);
    Serial.println("trying again....");
    
    if (aws.publish(MQTT_TOPIC, payload) == 0){// publishes payload and returns 0 upon success
      Serial.println("Success\n");
    }else{
      Serial.println("Failed on second attempt! moving on...\n");
    
    }
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

String setDelim () {
  String delim = ",";
  return delim;
}
