/**
 * This program was created by referring to BLE_Client.ino 
 * in the example sketch ESP32.
 * @author Minoru Inoue
 */

#include <M5Core2.h>
#include <Fonts/EVA_20px.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "BLEDevice.h"

// toioのサービスUUID(xxxx...には自分のtoioのUUIDを入れる)
static BLEUUID serviceUUID("xxxxxxxxxxxxxxxxxx");
// モーションセンサーのCharacteristic UUID
static BLEUUID    motionSensorUUID("10B20106-5B3B-4571-9508-CF3EFCD7BBAE");
// モーターのCharacteristic UUID
static BLEUUID    motorUUID("10B20102-5B3B-4571-9508-CF3EFCD7BBAE");

// 接続の状態
static boolean isDoConnect = false;
static boolean isConnected = false;
static boolean isDoScan = false;

// BLE操作用
static BLERemoteCharacteristic* pMotionSensorRemoteCharacteristic;
static BLERemoteCharacteristic* pMotorRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

// Notify通知時のコールバック
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {

    char str[25]="";
    char buf[4]="";
    
    Serial.print("モーションセンサの値: ");
    for(int i = 0;i<length ; i++){
      sprintf(buf,"%d,",(int)*(pData+i));
      strcat(str,buf);
    }
    showStr(str);
    Serial.println(str);
}

// BLEが接続された or 切れたときのコールバック
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    isConnected = true;
  }

  void onDisconnect(BLEClient* pclient) {
    isConnected = false;
  }
};

// UUID別にBLERemoteCharacteristicオブジェクトを作成する
BLERemoteCharacteristic* initChara(BLEClient* pClient, BLERemoteService* pRemoteService,BLEUUID characteristicUUID){
  
    pMotionSensorRemoteCharacteristic = pRemoteService->getCharacteristic(characteristicUUID);
    if (pMotionSensorRemoteCharacteristic == nullptr) {
      Serial.print("characteristic UUIDが見つかりませんでした: ");
      Serial.println(characteristicUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println("characteristic UUIDが見つかりました");

    if(pMotionSensorRemoteCharacteristic->canNotify())
      pMotionSensorRemoteCharacteristic->registerForNotify(notifyCallback);
   
    return pMotionSensorRemoteCharacteristic;
}

// 見つけたBLEデバイスの中にserviceUUIDと同じおのがあったときに接続します。
bool connectToServer() {
    Serial.print("接続を確立しています");
    Serial.println(myDevice->getAddress().toString().c_str());

    Serial.println("Client側を作成しています");
    BLEClient*  pClient  = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());

    // toioとつなげます
    Serial.println("toio(Server側)と接続します");
    pClient->connect(myDevice);
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    
    Serial.println("toioと接続できました");

    // モーションセンサを取得するためのオブジェクトを作成します。
    pMotionSensorRemoteCharacteristic = initChara(pClient, pRemoteService,motionSensorUUID);
    // モーターを操作するためのオブジェクトを作成します。
    pMotorRemoteCharacteristic = initChara(pClient, pRemoteService,motorUUID);

    isConnected = true;
    return true;
}

// BLEデバイスを検索するためのコールバック
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("Advertise中のデバイスがみつかりました:");
    Serial.println(advertisedDevice.toString().c_str());

    // toioのService UUIDを見つけたら操作用のオブジェクトを作成する
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      isDoConnect = true;
      isDoScan = true;

    }
  }
};

// 文字をM5Stack Core2に表示する
void showStr(char* atitudeStr){
 
  char Str[40];
  M5.Lcd.clear(WHITE);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.printf("DISPPLAY Test!");
  M5.Lcd.setCursor(10, 26);
  sprintf(Str,"Attitude:%s",atitudeStr);
  M5.Lcd.printf(Str);
}

// M5Stack Core2の画面を初期化します。
void screenInit(){
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setTextSize(2);
}

// 関数呼び出し時のCore2上のタッチパネル上のタッチ位置を取得する
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
// M5Stack Core2のタッチパネル位置をtoioの動作に変換する
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

// dataの中身をtoioのモータを停止させるものに初期化する
void dataInit(){
  data[0] = 0x01;
  data[1] = 0x01;
  data[2] = 0x01;
  data[3] = 0x00;
  data[4] = 0x02;
  data[5] = 0x01;
  data[6] = 0x00;
}

// M5Stack Core2上のタッチ位置取得とtoioのモータ操作を行う
void BLECheck() {

  // BLE機器の探索を開始している場合にserviceUUIDに示すデバイスとの接続を試みる
  if (isDoConnect == true) {
    if (connectToServer()) {
      Serial.println("toioとつながりました。");
    } else {
      Serial.println("toioとの接続に失敗しました。");
    }
    isDoConnect = false;
  }

  // toioとつながったら、液晶画面のタップ位置の取得とtoioへの操作を行う
  if (isConnected) {
    
    TouchPoint_t pos = touchflush();

    if(pos.x >= 0 && pos.y >= 0){
      makeMotorData(pos);
      pMotorRemoteCharacteristic->writeValue(data,7);
    }else{
      dataInit();
      pMotorRemoteCharacteristic->writeValue(data,7);
    }
    
  }else if(isDoScan){// toioを探す
    BLEDevice::getScan()->start(0); 
  }
}



void setup() {
  M5.begin(true, true, true, true);
  screenInit(); 
  Serial.println("BLEクライアントを作成します");
  BLEDevice::init("");

  // toioとの接続をこころみる
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  dataInit();
  Serial.println("toioとの接続と設定が完了しました");
}


void loop() {
  BLECheck();
  
  delay(10);
}
