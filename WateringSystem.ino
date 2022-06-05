#include <ArduinoJson.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <FTPClient_Generic.h>
#include "secret_define.h"

#define DEBUG_MSG 1 //シリアルで処理メッセージを出力するためのフラグ

#define JST     3600*9

extern "C" {
  #include "user_interface.h"
}

const char* ssid     = MYSSID;
const char* password = SSID_PASS;

const char* host = FTP_SERVER;
const char* url = CONFIG_FILE;
const uint16_t port = 80;

//FTPサーバーと接続するための設定
char ftp_server[] = FTP_SERVER;
char ftp_user[]   = FTP_USER;
char ftp_pass[]   = FTP_PASS;
char dirName[]    = FTP_DIR;
FTPClient_Generic ftp (ftp_server, ftp_user, ftp_pass, 60000);


int year = 1970;
int month = 1;
int day = 1;
int hour = 0;
int minute = 0;
int sec = 0;
int span = 24;
int ml = 0;


struct tm tm_base;
time_t t_base;
int sec_span = span * 3600; // 水やりの時間(jsonファイルをダウンロードして更新する)
int sec_loop = 1 * 3600; // 死活ログの時間(秒)
//int sec_loop = 10; // 死活ログの時間(秒) // デバッグ用


const int motorPin =  12;      // the number of the MOTOR pin


// サーバーに設定を見に行く
// うまく見に行ければTrue、失敗したらFalse
// ディープスリープの復帰後に毎回実施
bool checkConfig(){
  // Use WiFiClient class to create TCP connections
  // Connect to HTTP server
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
    return false;
  }
  if (client.connected()) {
    Serial.println(F("Connected!"));
  }
  // jsonファイルをダウンロードする
  // Send HTTP request
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" +
             "Connection: close\r\n\r\n");
  if (client.println() == 0) {
    Serial.println("Failed to send request");
    client.stop();
    return false;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
  if (strcmp(status + 9, "200 OK") != 0) {
    Serial.print("Unexpected response: ");
    Serial.println(status);
    client.stop();
    return false;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("Invalid response");
    client.stop();
    return false;
  }

  // Allocate the JSON document
  // Use https://arduinojson.org/v6/assistant to compute the capacity.
  const size_t capacity = JSON_OBJECT_SIZE(8) + JSON_ARRAY_SIZE(0) + 60;
  DynamicJsonDocument doc(capacity);

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    client.stop();
    return false;
  }

  // 正常にconfig.jsonの取得が完了
  // Disconnect
  client.stop();

  // Extract values
  year = doc["year"].as<long>();
  month = doc["month"].as<long>();
  day = doc["day"].as<long>();
  hour = doc["hour"].as<long>();
  minute = doc["min"].as<long>();
  sec = doc["sec"].as<long>();
  span = doc["span"].as<long>();
  ml = doc["ml"].as<long>();
  sec_span = span * 3600;
  //sec_span = 10;


  //jsonから読み込んだ設定情報を変数に代入する。
  tm_base.tm_year = year - 1900; /* 西暦年 - 1900 */
  tm_base.tm_mon = month - 1; /* 月 ( 1月＝0 ) */
  tm_base.tm_mday = day;
  tm_base.tm_hour = hour;
  tm_base.tm_min = minute;
  tm_base.tm_sec = sec;
  tm_base.tm_isdst = -1; /* サマータイムフラグ */
  t_base = mktime(&tm_base);//JST
  
#if DEBUG_MSG
  Serial.println("config.json:");
  Serial.println(year);
  Serial.println(month);
  Serial.println(day);
  Serial.println(hour);
  Serial.println(minute);
  Serial.println(sec);
  Serial.println(span);
  Serial.println(ml);
#endif

  return true;
}

// WiFiへの接続
// ディープスリープの復帰後に毎回実施
void connectWiFi(){
#if DEBUG_MSG
  Serial.print("Connecting to ");
  Serial.println(ssid);
#endif
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //WiFiに接続できていなければ待つ
  //接続完了したら抜ける
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
#if DEBUG_MSG
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif  
}

// 起動時と、ディープスリープ復帰後に実施する処理
void postDS(){
  //スリープモードの選択
  //    wifi_set_sleep_type(NONE_SLEEP_T);
  //    wifi_set_sleep_type(MODEM_SLEEP_T);
  wifi_set_sleep_type(NONE_SLEEP_T);
  // WiFiに接続する
  connectWiFi();
  // 設定ファイルをダウンロードする
  bool flg = checkConfig();
  if(flg){
    Serial.println("Online Mode");  
  }else{
    Serial.println("Offline Mode");  
  }
  // 時計をJST基準にする
  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
  delay(2000);// 少し待たないと更新されない  
  wifi_set_sleep_type(LIGHT_SLEEP_T);

}


void setup() {
  // initialize the MOTOR pin as an output:
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW);
  // Initialize Serial port
  Serial.begin(9600);
  while (!Serial) continue;
  //WiFi処理
  postDS();
  
  // 残り時間計算
  time_t t;
  struct tm *tm;
  t = time(NULL);//JST
  tm = localtime(&t);//JST
  int t_remain = (59 - tm->tm_min) * 60 + (60 - tm->tm_sec);
  Serial.printf("%d %04d/%02d/%02d %02d:%02d:%02d\n",
        t_remain, tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec);
  Serial.printf("Sleep %d seconds\n", t_remain);

  //デバッグ用
  Watering(ml);
  logWater(tm);
  logAlive(tm);
  //delay(t_remain * 1000);//本番用
  delay(30 * 1000); // デバッグ用
}

void Watering(int ml){
  
  Serial.println("Watering");  

  //水の量に応じて、delayの時間を調整する。
  //3V
  //1s:0ml
  //2s:14ml
  //3s:45ml
  //4s:77ml
  //5s:104ml
  //x = (t - 1.5) * 30
  //t = x / 30 + 1.5
  int t = int(((double)ml / 30 + 1.5) * 1000);
  Serial.printf("water %d %d\n",
      t, ml);
  if(t < 0)t = 0;
  digitalWrite(motorPin, HIGH);
  delay(t);
  digitalWrite(motorPin, LOW);
  return;
}
void logWater(tm *tm){
  // ファイルに現在時刻を追記する
  // 年,月,日,時,分,秒,ml
  // 

  Serial.printf(" %04d/%02d/%02d %02d:%02d:%02d\n",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec);
  
  Serial.println("Logging");  

  char value[100];
  sprintf(value, "%04d/%02d/%02d %02d:%02d:%02d\n",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec);
  ftp.OpenConnection();

  //Change directory
  ftp.ChangeWorkDir(dirName);

  
  ftp.InitFile(COMMAND_XFER_TYPE_ASCII);
  ftp.NewFile("water.txt");
  ftp.Write(value);
  ftp.CloseFile();

  
  return;
}
void logAlive(tm *tm){
  // ファイルに現在時刻を追記する
  // 年,月,日,時,分,秒,ml
  // 
  Serial.printf(" %04d/%02d/%02d %02d:%02d:%02d\n",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec);
  
  Serial.println("Alive");  
  
  char value[100];
  sprintf(value, "%04d/%02d/%02d %02d:%02d:%02d\n",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec);
  ftp.OpenConnection();

  //Change directory
  ftp.ChangeWorkDir(dirName);

  
  ftp.InitFile(COMMAND_XFER_TYPE_ASCII);
  ftp.NewFile("alive.txt");
  ftp.Write(value);
  ftp.CloseFile();
  
  return;
}
void loop() {
  //WiFi処理
  postDS();
  
  // 水やり時間のチェック
  time_t t;
  struct tm *tm;
  t = time(NULL);//JST
  tm = localtime(&t);//JST
  
  int elapsed = int(difftime(t, t_base)); //configの設定時間からの経過時間(秒)
  Serial.printf("elapsed time from config time : %d[s]\n", elapsed);
  Serial.printf("elapsed time from scheduled time : %d[s]\n", elapsed % sec_span);
  // もし経過時間が周期を超えたら、水やりをする
  // 水やり予定時間の20分前後だったら、水やりを実行する（このチェックが1時間に1回なので、この幅で大丈夫）

  if((elapsed % sec_span < (20 * 60)) || (elapsed % sec_span > (sec_span - 20*60))){
    // 水やり
    Watering(ml);
    // ログ
    logWater(tm);
  }

  // 死活報告
  logAlive(tm);

  // 残り時間を確認してリープ
  t = time(NULL);//JST
  tm = localtime(&t);//JST
  int t_remain = (59 - tm->tm_min) * 60 + (60 - tm->tm_sec);
  Serial.printf("Sleep %d seconds\n", t_remain);
  //delay(t_remain * 1000);//本番用
  delay(30 * 1000); // デバッグ用

}
