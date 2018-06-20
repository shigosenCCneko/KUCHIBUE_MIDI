# KUCHIBUE_MIDI

## FFTクチブエtoMIDIコンバーターfor AVR

Atmega328を使いマイクから入力された口笛をFFTにより
シリアルからMIDI出力するArduinoIDEのスケッチです。

### 入力
PC0(ArduinoならアナログPIN0)へマイク入力を繋いでください  
入力の閾値は#define THRESHOLD_LEVELで調整します 
テストには秋月のコンデンサマイク(アンプ付き)キットを使用しました

### 出力
MIDIのノートオン、ノートオフのみシリアルから出力します
BaudRateはsetup()の中でSerial.begin()で設定しています。
音域はA#2からC5までに対応しています

### プログラムについて
サンプリングレート2.2Khz(452us)で256STEP時間窓は115ms(8Hz)
28.75msごとに交互に解析しながらタイマ割り込みを使用してサンプリングしています。  
FFTのプログラムはArduinoのライブラリのfixfttを256step固定の  
リバース無しで計算の一部をテーブル化などを行い高速化しました。　　

