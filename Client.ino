#include <M5Core2.h>
#include <Fonts/EVA_20px.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "BLEDevice.h"
//#include "BLEScan.h"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("xxxxxxxxxxxxxxxxxxxxxxx");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("10B20106-5B3B-4571-9508-CF3EFCD7BBAE");
static BLEUUID    motorUUID("10B20102-5B3B-4571-9508-CF3EFCD7BBAE");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLERemoteCharacteristic* pRemoteCharacteristic2;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {

    char str[25]="";
    char buf[4]="";
    
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    for(int i = 0;i<length ; i++){
      sprintf(buf,"%d,",(int)*(pData+i));
      strcat(str,buf);
    }
    showStr(str);
    Serial.println(str);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

BLERemoteCharacteristic* initChara(BLEClient* pClient, BLERemoteService* pRemoteService,BLEUUID charUUID_){
  
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID_);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID_.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    
    return pRemoteCharacteristic;
}

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    pRemoteCharacteristic = initChara(pClient, pRemoteService,charUUID);
    pRemoteCharacteristic2 = initChara(pClient, pRemoteService,motorUUID);

    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void showStr(char* atitudeStr){
 
  char Str[40];
  M5.Lcd.clear(WHITE);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.printf("DISPPLAY Test!");
  M5.Lcd.setCursor(10, 26);
  sprintf(Str,"Attitude:%s",atitudeStr);
  M5.Lcd.printf(Str);
}

void screenInit(){
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setTextSize(2);
}


TouchPoint_t touchflush()
{   
    M5.Lcd.setCursor(10, 10);
    TouchPoint_t pos= M5.Touch.getPressPoint();
    bool touchStateNow = ( pos.x == -1 ) ? false : true;
    if( touchStateNow )
    {
      char buf[20] = "";
      char str[60] = "";
      
      sprintf(buf,"TOUCH:%d,",(int)pos.x);
      strcat(str,buf);
      sprintf(buf,"%d,",(int)pos.y);
      strcat(str,buf);
      Serial.println(str);
      return pos;
    }

    pos.x = -1;
    pos.y = -1;
    return pos;
}

float satNum(float num){
  if(num > 0.5){
    return 0.5;
  }
  return num;
}

uint8_t data[7];
void makeMotorData(TouchPoint_t pos){
  float normalX = (float)pos.x/320;
  float normalY = (float)pos.y/280;

  float normalLeftWheelPower = ((-normalY+0.5)*2)*(satNum(normalX)*2);
  float normalRightWheelPower = ((-normalY+0.5)*2)*(satNum(1-normalX)*2);

  uint8_t leftWay=0x01;
  uint8_t rightWay=0x01;

  if(normalLeftWheelPower<0){
    leftWay=0x02;
  }
  if(normalRightWheelPower<0){
    rightWay=0x02;
  }

  uint8_t leftWheelPower = (uint8_t)abs(normalLeftWheelPower*100);
  uint8_t rightWheelPower = (uint8_t)abs(normalRightWheelPower*100);
  
  data[0] = 0x01;
  data[1] = 0x01;
  data[2] = leftWay;
  data[3] = leftWheelPower;
  data[4] = 0x02;
  data[5] = rightWay;
  data[6] = rightWheelPower;
}

void dataInit(){
  data[0] = 0x01;
  data[1] = 0x01;
  data[2] = 0x01;
  data[3] = 0x00;
  data[4] = 0x02;
  data[5] = 0x01;
  data[6] = 0x00;
}

void BLECheck() {
    // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    String newValue = "Time since boot: " + String((millis())/1000);
    //Serial.println("Setting new characteristic value to \"" + newValue + "\"");
    TouchPoint_t pos = touchflush();

    if(pos.x >= 0 && pos.y >= 0){
      makeMotorData(pos);
      pRemoteCharacteristic2->writeValue(data,7);
    }else{
      dataInit();
      pRemoteCharacteristic2->writeValue(data,7);
    }
    // Set the characteristic's value to be the array of bytes that is actually a string.
    //pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }
}



void setup() {
  //Serial.begin(115200);
  M5.begin(true, true, true, true);
  screenInit(); 
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  dataInit();
  Serial.println("Setting Done");
} // End of setup.


// This is the Arduino main loop function.
void loop() {
  BLECheck();
  
  delay(10);
} // End of loop
