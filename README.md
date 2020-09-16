# M5StackCore2_to_Toio
M5Stack Core2とtoioをBLE経由でつなげてラジコンっぽく遊べるプログラムです。


このプログラムはスケッチ例のESP32のBLE_Client.inoを参照して作成されました。
This program was created by referring to BLE_Client.ino in the example sketch ESP32.

また、本プログラムは下記リンクで公開されているtoioの技術仕様を元にした非公式な実装です。
This program is an unofficial implementation based on the technical specification of toio, which is published in the following link.

https://toio.github.io/toio-spec/

Please see [LICENSE](LICENSE) .

## 接続方法
Client.inoにあるserviceUUIDにご自身のtoioのService UUIDをいれてからM5Stack Core2に書き込んでください。
書き込み終わったらtoioに電源を入れて、M5Stack Core2のリセットボタンを押してください。toioの接続音が確認できればOKなはずです。

## 操作方法(先にtoioとの接続を完了してください)
M5Stack Core2の画面中央から上を触るとtoioが前進し、下を触ると後退します。
このとき、画面右側へ指をずらすと右折、画面左側にずらすと左折します。


toioをタップしたり、ひっくり返したりすると画面にtoioの姿勢に関する数値情報が表示されます。
表示されている数値の意味は下記リンクを参照してください。

https://toio.github.io/toio-spec/docs/ble_sensor
