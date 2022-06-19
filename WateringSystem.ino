#include <ArduinoJson.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <FTPClient_Generic.h>
#include <ESP8266WebServer.h>
#include "secret_define.h"

#define DEBUG_MSG 1 //シリアルで処理メッセージを出力するためのフラグ

#define JST     3600*9

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
int min_alive = 5; // 死活ログの時間(分)
bool flg_connected = false;//起動から、一度WIFIがつながっていたら、WIFI設定モードに入らないようにするためのフラグ。

const int motorPin =  12;      // the number of the MOTOR pin

// Wi-Fi設定保存ファイル
const char* settings = "/wifi_settings.txt";
// サーバモード起動時のパスワード
const String pass = ESP_PASS;
ESP8266WebServer server(80);

/**
 * WiFi設定画面
 */
void handleRootGet() {
  String html = "";
  html += "<h1>WiFi Settings</h1>";
  html += "<form method='post'>";
  html += "  <input type='text' name='ssid' placeholder='ssid'><br>";
  html += "  <input type='text' name='pass' placeholder='pass'><br>";
  html += "  <input type='submit'><br>";
  html += "</form>";
  server.send(200, "text/html", html);
}
void handleRootPost() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");

  File f = SPIFFS.open(settings, "w");
  f.println(ssid);
  f.println(pass);
  f.close();

  String html = "";
  html += "<h1>WiFi Settings</h1>";
  html += ssid + "<br>";
  html += pass + "<br>";
  server.send(200, "text/html", html);
}

/**
 * クライアントモードで設定ファイルを読み込み。
 */
void setup_client() {

  File f = SPIFFS.open(settings, "r");
  String ssid = f.readStringUntil('\n');
  String pass = f.readStringUntil('\n');
  f.close();

  ssid.trim();
  pass.trim();

  Serial.println("SSID: " + ssid);
  Serial.println("PASS: " + pass);

  //WIFI ON
  WiFi.forceSleepWake();
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid.c_str(), pass.c_str());
  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    cnt += 1;
    if(cnt > 30){//接続がうまく行かなかった場合。
      Serial.println("");
      Serial.println("WiFi not connected");
      break;
    }
  }
  if(cnt <= 30){ //接続がうまく行った場合。
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

/**
 * サーバモードで起動。
 * SSIDはMACアドレス、パスワードは設定したもの。URLは192.168.4.1(つながらない場合は、設定用PCのIPアドレスの上位3セグメントを参照)
 */
void setup_server() {
  byte mac[6];
  WiFi.macAddress(mac);
  String ssid = "";
  for (int i = 0; i < 6; i++) {
    ssid += String(mac[i], HEX);
  }
  Serial.println("SSID: " + ssid);
  Serial.println("PASS: " + pass);
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid.c_str(), pass.c_str());
  server.on("/", HTTP_GET, handleRootGet);
  server.on("/", HTTP_POST, handleRootPost);
  server.begin();
  Serial.println("HTTP server started.");
}

// サーバーに設定を見に行く
// うまく見に行ければTrue、失敗したらFalse
// スリープの復帰後に毎回実施
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

// 起動時と、スリープ復帰後に実施する処理
void postDS(){
  //スリープモードの選択
  //    wifi_set_sleep_type(NONE_SLEEP_T);
  //    wifi_set_sleep_type(MODEM_SLEEP_T);
  wifi_set_sleep_type(NONE_SLEEP_T);
  // WiFiに接続する
  setup_client();
  // 設定ファイルをダウンロードする
  bool flg = checkConfig();
  if(flg){
    Serial.println("Online Mode");  
  }else if(!flg_connected){
    Serial.println("Offline Mode");  
    // サーバモードに入る
    setup_server();
    while(1){
      server.handleClient();
    }
  }else if(flg_connected){//一度WIFIには接続されていて、設定ファイルをダウンロードできなかった場合は、ダウンロードリトライする。
    while(1){
      // WiFiに接続する
      setup_client();
      // 設定ファイルをダウンロードする
      flg = checkConfig();
    }
  }
  // 時計をJST基準にする
  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
  delay(2000);// 少し待たないと更新されない  

}


void setup() {
  // initialize the MOTOR pin as an output:
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW);
  // Initialize Serial port
  Serial.begin(9600);
  while (!Serial) continue;

  // ファイルシステム起動
  SPIFFS.begin();

  //WiFi設定＆Config読み込み処理
  postDS();
  flg_connected = true;//ここまできているということは、一度WIFIにつながったということ。
  
  // 残り時間計算
  time_t t;
  struct tm *tm;
  t = time(NULL);//JST
  tm = localtime(&t);//JST
  int t_remain = (min_alive - tm->tm_min % min_alive) * 60 - tm->tm_sec;
  Serial.printf("%d %04d/%02d/%02d %02d:%02d:%02d\n",
        t_remain, tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec);
  Serial.printf("Sleep %d seconds\n", t_remain);

  //デバッグ用
  Watering(ml);
  logWater(tm);
  logAlive(tm);


  // WIFI OFF
  WiFi.disconnect();
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  Serial.println("WiFi is down");
  
  delay(t_remain * 1000);//本番用
  //delay(30 * 1000); // デバッグ用
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
  
  unsigned long long int elapsed = (unsigned long long int)(difftime(t, t_base)); //configの設定時間からの経過時間(秒)
  Serial.printf("elapsed time from config time : %ld[s]\n", elapsed);
  Serial.printf("elapsed time from scheduled time : %ld[s]\n", elapsed % sec_span);
  // もし経過時間が周期を超えたら、水やりをする
  // 水やり予定時間の前後だったら、水やりを実行する
  if((elapsed % sec_span < (min_alive * 60 * 2)) && (elapsed % sec_span >= 0)){
    // 水やり
    Watering(ml);
    // ログ
    logWater(tm);
  }

  // 死活報告
  logAlive(tm);


  // WIFI OFF
  WiFi.disconnect();
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  Serial.println("WiFi is down");

  // 残り時間を確認してリープ
  t = time(NULL);//JST
  tm = localtime(&t);//JST
  int t_remain = (min_alive - tm->tm_min % min_alive) * 60 - tm->tm_sec;
  Serial.printf("Sleep %d seconds\n", t_remain);
  delay(t_remain * 1000);//本番用
  //delay(30 * 1000); // デバッグ用

}
