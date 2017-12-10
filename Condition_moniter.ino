#include <SPI.h>
#include <Ethernet.h> // Ethernetシールド(W5100)用ライブラリ
#include "Ambient.h"  // Ambient.ioに接続するためのライブラリ
#include "DHT.h"  // 温湿度センサDHT11用ライブラリ
#define DHTTYPE DHT11 // 使用するセンサーのタイプを設定 DHT 11 "DHT.h"

// 使用するピンの明記
#define DHTPIN A0 // A0 温湿度センサDHT11
#define MOISPIN A1  //int sensorPin = A0; // select the input pin for the potentiometer
#define LED 13

// シリアル通信用の定義
#define _DEBUG 1  //_DEBUGを1と定義
#if _DEBUG
#define DBG(...) { Serial.print(__VA_ARGS__); } // DBG(引数)をSerial.print(引数)と定義
#define DBGLED(...) { digitalWrite(__VA_ARGS__); }
#else
#define DBG(...)
#define DBGLED(...)
#endif /* _DBG */

#define PERIOD 30

// WatchDogタイマーに使用する変数
int wdt_counter = 0;  //スリープの長さを設定するためのカウントアップ変数

// EthernetシールドのMACアドレスの設定
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Ambientのチャンネル設定
unsigned int channelId = 2797;  // unsigned intは符号なし整数。Ambient.ioのチャンネルIDを入力
const char* writeKey = "b31446b3fb805927";  //上記のライトキーを入力

//外部ライブラリーのオブジェクトを生成
EthernetClient client;  //Ethernetクラスのclientオブジェクトを生成
Ambient ambient;  //ambientオブジェクトの生成
DHT dht(DHTPIN, DHTTYPE); //dhtオブジェクトの生成


void setup()
{
  #ifdef _DEBUG //_DEBUGが1の場合
    Serial.begin(9600);
    delay(20);
  #endif

  pinMode(LED, OUTPUT); // 13番ピンをLEDに設定

  //各クラスの開始
  Ethernet.begin(mac);  //Ethernetクライアントの開始
  ambient.begin(channelId, writeKey, &client); //Ambient.beginを呼び出し。&はアドレス演算子。clientのアドレスを引数として渡すことで、関数が変数の中身を変更することができる
  dht.begin();

  //インターネット接続状況の表示
  DBG("Start");  //コンパイル時にSerial.print("Start")に置き換えられる
  DBG("Internet connected\r\n");
  DBG("IP address: ");
  DBG(Ethernet.localIP());
  DBG("\r\n");
}


void loop()
{
  do {
    WDT_sleep();
    } while (wdt_counter < 37);  //296秒(8秒×37回,約5分)経過するまでスリープ
  WDT_stop(); //スリープを解除
  wdt_counter = 0;  //スリープ用カウントアップ変数を初期化
  
  //ここから処理を記載
  digitalWrite(LED,HIGH);
  delay(1000);
  digitalWrite(LED,LOW);
  delay(1000);

  // 温湿度センサの値を取得
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  char humidbuf[12];
  
  //土壌水分センサーの値を取得
  int soilmoist = analogRead(MOISPIN);

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(temp) || isnan(humid)) 
  {
    DBG("Failed to read from DHT");
  } 
    else 
    {
      DBG("Temperatyre: ");
      DBG(temp);
      DBG(" DegC, Humidity: ");
      DBG(humid);
      DBG("Soil Moisture: ");
      DBG(soilmoist);
      DBG(" %\r\n");
    }

  //Ambientサーバーにデータを送信
  ambient.set(1, temp);                // データーがint型かfloat型であれば、直接セットすることができます。
  dtostrf(humid, 3, 1, humidbuf);      // データーの桁数などを制御したければ自分で文字列形式に変換し、
  ambient.set(2, humidbuf);            // セットします。
  ambient.set(3, soilmoist);
  ambient.send();

}

//watchdog sleep
void WDT_sleep(){
    // set power-down mode
  SMCR |= (1 << SM1); //パワーダウンモードに設定
  SMCR |= 1; //スリープ機能の許可
  
  // disable ADC アナログ入力をデジタル値に変換するA/D変換機能をOFFに設定
  ADCSRA &= ~(1 << ADEN); //OFF

  // set watchdog timer ウォッチドッグタイマの設定
  asm("wdr"); // ウォッチドッグタイマをリセット
  WDTCSR |= (1 << WDCE)|(1 << WDE); // ウォッチドッグタイマ設定変更のため，WDCEとWDEに同時に1を出力
  WDTCSR = (1 << WDIE)|(1 << WDP3)|(1 << WDP0); // 8 sec, interrupt タイマの時間と時間経過時の動作を設定
  //WDTCSR  = (1 << WDIE) | (1 << WDP2) | (1 << WDP0); // 0.5 sec

  // disable BOD アナログ入力をデジタル値に変換するA/D変換機能をOFFに設定
  MCUCR |= (1 << BODSE)|(1 << BODS); // BOD設定変更のため，BODSとBODSEに同時に1を出力
  MCUCR = (MCUCR & ~(1 << BODSE))|(1 << BODS); // BODSに1，BODSEに0を出力してBODをOFFにする
  asm("sleep"); //スリープ状態へ移行
}

//watchdog stop
void WDT_stop(){
  // stop watchdog timer
  asm("wdr");
  MCUSR &= ~(1 << WDRF);
  WDTCSR |= (1 << WDCE)|(1 << WDE);
  WDTCSR = 0;

  // enable ADC
  ADCSRA |= (1 << 7);
}

//watchdog interrupt
ISR(WDT_vect){
  wdt_counter++;
}
