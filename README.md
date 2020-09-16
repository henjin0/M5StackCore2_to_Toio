# M5StackCore2_to_Toio
M5Stack Core2とtoioをBLE経由でつなげてラジコンっぽく遊べるプログラムです。

This program was created by referring to BLE_Client.ino in the example sketch ESP32.
Please see LICENSE .

## 接続方法
Client.inoにあるserviceUUIDにご自身のtoioのService UUIDをいれてからM5Stack Core2に書き込んでください。
書き込み終わったらtoioに電源を入れて、M5Stack Core2のリセットボタンを押してください。toioの接続音が確認できればOKなはずです。

