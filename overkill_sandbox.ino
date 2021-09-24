#include "bms.h"
#include "options.h"
#include "WiFi.h"
#include "HTTPClient.h"

const char* host = "https://postb.in/1631759911516-7197994671296";

OverkillSolarBms bms = OverkillSolarBms();
uint32_t last_soc_check_time;

#define SOC_POLL_RATE 2000  // milliseconds

HardwareSerial HWSerial(2); // Define a Serial port instance called 'Receiver' using serial port 2

#define Receiver_Txd_pin 17
#define Receiver_Rxd_pin 16

void setup() {
    static const uint8_t LED_BUILTIN = 2;
    // Connect to BMS
    pinMode(LED_BUILTIN, OUTPUT);
    //Serial1.begin(9600);

    Serial.begin(115200);                                                   // Define and start serial monitor
    HWSerial.begin(9600, SERIAL_8N1, Receiver_Txd_pin, Receiver_Rxd_pin);
    
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

}

void loop() {
    bms.main_task();
    bms.debug();
    // uint8_t soc = bms.get_state_of_charge();

    // if (millis() - last_soc_check_time > SOC_POLL_RATE) {

    //     /**
    //     Gether up data from the bms
    //     */

    //     // Get voltage
    //     float voltage = bms.get_voltage();
    //     // Serial.print("Voltage: ");
    //     // Serial.print(voltage, 3);
    //     // Serial.println(" volts");
        
    //     // Get current
    //     float current = bms.get_current();
    //     // Serial.print("Current:" );
    //     // Serial.print(current, 1);
    //     // Serial.println(" amps");      

    //     // Get voltage diff
    //     float lowest = 99;
    //     float highest = 0;
    //     float v = 0;
    //     for(int i=0; i < 16; i++){
    //       v = bms.get_cell_voltage(i);
    //       if(v < lowest){
    //         lowest = v;
    //       }
    //       if(v > highest){
    //         highest = v;
    //       }
    //     } 
    //     float voltageDiff = highest - lowest;
        
    //     // Get state of charge
    //     uint8_t soc = bms.get_state_of_charge();
        
    //     // Get charge fet status
    //     bool cFet = bms.get_charge_mosfet_status() ? "on": "off";

    //     // Get discharge fet status
    //     bool dFet = bms.get_discharge_mosfet_status() ? "on": "off";
      
    //     // Get flags
    //     ProtectionStatus flags = bms.get_protection_status();
    //     String flagString = "{";
    //     flagString += (String)"single_cell_overvoltage_protection" + ":" + flags.single_cell_overvoltage_protection;
    //     flagString += (String)",single_cell_undervoltage_protection" + ":" + flags.single_cell_undervoltage_protection;
    //     flagString += (String)",whole_pack_overvoltage_protection" + ":" + flags.whole_pack_overvoltage_protection;
    //     flagString += (String)",whole_pack_undervoltage_protection" + ":" + flags.whole_pack_undervoltage_protection;
    //     flagString += (String)",charging_over_temperature_protection" + ":" + flags.charging_over_temperature_protection;
    //     flagString += (String)",charging_low_temperature_protection" + ":" + flags.charging_low_temperature_protection;
    //     flagString += (String)",discharge_over_temperature_protection" + ":" + flags.discharge_over_temperature_protection;
    //     flagString += (String)",discharge_low_temperature_protection" + ":" + flags.discharge_low_temperature_protection;
    //     flagString += (String)",charging_overcurrent_protection" + ":" + flags.charging_overcurrent_protection;
    //     flagString += (String)",discharge_overcurrent_protection" + ":" + flags.discharge_overcurrent_protection;
    //     flagString += (String)",short_circuit_protection" + ":" + flags.short_circuit_protection;
    //     flagString += (String)",front_end_detection_ic_error" + ":" + flags.front_end_detection_ic_error;
    //     flagString += (String)",software_lock_mos" + ":" + flags.software_lock_mos;
    //     flagString += "}";

    //     // Get bms name
    //     String name = bms.get_bms_name();

    //     // Build the json we'll send
    //     String jsonStr = "{";
    //     jsonStr += "name:" +(String)name;
    //     jsonStr += ",voltage:" + (String)voltage;
    //     jsonStr += ",current:" + (String)current;
    //     jsonStr += ",voltageDiff:" + (String)voltageDiff;
    //     jsonStr += ",soc:" + (String)soc;
    //     jsonStr += ",cFet:" + (String)cFet;
    //     jsonStr += ",dFet:" + (String)dFet;
    //     jsonStr += ",lowestCell:" + (String)lowest;
    //     jsonStr += ",highestCell:" + (String)highest;

    //     Serial.print(jsonStr);
    //     Serial.println();
    //     /*
    //     Serial.print("Voltage: ");
    //     Serial.print(voltage, 3);
    //     */
        
    //     // record the time before we hit the server
    //     last_soc_check_time = millis();

    //     /**
    //      Send the data to the server
    //      */
    //     //postData();
    // }
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
