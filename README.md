# KUCHIBUE_MIDI

## FFTクチブエtoMIDIコンバーターfor AVR

Atmega328を使いマイクから入力された口笛をFFTで解析しシリアルからMIDIのNOTEオンNOTEオフとして出力するArduinoIDEのスケッチです。

### 入力とFFT変換
CRのローパスフィルタとOPアンプのハイパスフィルタを通して
PC0(ArduinoならアナログPIN0)でAD変換した値をサンプリングレートの異なる2つのリングバッファへ保存し、高音と低音用のFFT解析用のバッファを2組ずつ用意して、解析中に他のバッファの組へデータを転送しています。  
入力の閾値は#define THRESHOLD_LEVELで調整します
マイク入力には秋月のコンデンサマイク(アンプ付き)キットを使用しました

### 出力
MIDIのノートオン、ノートオフのみシリアルから出力します  
BaudRateはsetup()の中でSerial.begin()で2種類設定し起動時にPB3の入力で選択されます。  
音域の入力はA#2からC5までに対応しておりoctave_upを利用してC6まで出力できます。


### コマンドその他
- PB0～PB2ピンをGNDへ落とすことにより以下の機能を割り当てています

- PB0 PROGRAM CHANGE　(YMF825MIDI_CONTROL_for_Arduinoのみ)音色変更のシステムエクルシーブメッセージをYMF825へ送ります  
PB1 octave_up GNDへ落としている間1オクターブ高いNOTEオンを出力します
- PB3 リセット後のUARTの速度選定,シリアル出力をUSBtoSerialデバイスなどでPCへ送りSerialMIDI変換ソフトでMIDI入力として使用出来ます。

### FFTのサンプルレート等
時間窓長とサンプリングレートは分解能と処理速度が問題となり、試行錯誤の末に、Step数128,サンプリング間隔226uSと452uS,時間窓長28.9ms(34.6Hz)と57.9ms(17.3Hz)の解析用バッファの組を交互に解析と転送に用いながら、14.5msごとに解析を行っています。  
FFTのプログラムはArduinoのライブラリのfixfttを参考に128Step固定,リバース無しで計算の一部をテーブル化などを行い高速化しました。　　
### 入力音(口笛)の変換について
口笛に関しては綺麗な音で鳴らすより、多少濁っても音程が正しく吹くようにしてください、一音を一定の息遣いで鳴らすようにすれば連続音としてなりますが音が乱れると連符になってしまいます。  
KUCHIBUE-MIDI-AVR.inoでFFTのピーク値を変換テーブル  
(note_no[],note_no_lo[])を用いMIDIのNOTE_NOへ変換しているのですが、癖に合わせてここを変更すれば認識率が上がると思います。
