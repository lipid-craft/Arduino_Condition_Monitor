/*
 * ambient.h - Library for sending data to Ambient
 * Created by Takehiko Shimojima, April 21, 2016
 */
#ifndef Ambient_h //条件コンパイルによる重複定義の回避回避 #ifndefから#endif
#define Ambient_h

#include "Arduino.h"
//#include <ESP8266WiFi.h>
#include <Ethernet.h>

#define AMBIENT_WRITEKEY_SIZE 18
#define AMBIENT_MAX_RETRY 5
#define AMBIENT_DATA_SIZE 24
#define AMBIENT_NUM_PARAMS 11
#define AMBIENT_TIMEOUT 3000 // milliseconds

class Ambient   //構造体の宣言
{
public: //メンバ関数の宣言

    Ambient(void);

//    bool begin(unsigned int channelId, const char * writeKey, WiFiClient * c, int dev = 0);  //メンバ関数の宣言 bool型は論理 True又はFalseを返す//writeKey cはポイント型変数。変数の前に*を付けて宣言する
    bool begin(unsigned int channelID, const char * writeKey, EthernetClient * c, int dev = 0);
    bool set(int field,const char * data); // *変数はポインタの宣言
	  bool set(int field, double data);
	  bool set(int field, int data);
    bool clear(int field);

    bool send(void);
    int bulk_send(char * buf);
    bool delete_data(const char * userKey);

private: //データメンバの宣言

//    WiFiClient * client;  //構造体WiFiClientでポインタclientを宣言
    EthernetClient * client;
    unsigned int channelId;
    char writeKey[AMBIENT_WRITEKEY_SIZE];
    int dev;
    char host[18];
    int port;

    struct {  //データ構造 structの宣言
        int set;
        char item[AMBIENT_DATA_SIZE];
    } data[AMBIENT_NUM_PARAMS];
};

#endif // Ambient_h
