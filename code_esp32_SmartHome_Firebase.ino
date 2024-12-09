#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/RTDBHelper.h>
#define DATABASE_URL ""

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;
char ssid[] = "";
char pass[] = "";

#include <Arduino.h>
#include <Ticker.h>
Ticker ticker;
Ticker ckFinger;
#define BUZZ 5

#include <SoftwareSerial.h>
SoftwareSerial lcd(25,26);
SoftwareSerial STM(27,14);

#include <Adafruit_Fingerprint.h>
#define mySerial Serial2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
int id=0; 

#include "DHT.h"
DHT dht1(18, DHT11);
DHT dht2(19, DHT11);
DHT dht3(21, DHT11);
int gas1=0, gas2=0, gas3=0;
float t1,h1,t2,h2,t3,h3;
int st1=0, st2=0, st3=0, st4=0, st5=0, st6=0;
int sp1=0, sp2=0, sp3=0, sp4=0, sp5=0, sp6=0;
int checkWarn = 0;
int ckID = -2;

void blink() {
  digitalWrite(BUZZ, !digitalRead(BUZZ));
}
void tick(int x=1, int y=90)
{
  while (x--)
  {
    digitalWrite(BUZZ,1); delay(y);
    digitalWrite(BUZZ,0); delay(y);
  }
}
void getIDfinger()
{
  ckFinger.detach();
  if (ckID <= 0)
  {
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK)  ckID = -2;
    else
    {
      p = finger.image2Tz();
      if (p != FINGERPRINT_OK)  ckID = -1;
      else
      {
        p = finger.fingerFastSearch();
        if (p != FINGERPRINT_OK)  ckID = 0;
        else ckID = finger.fingerID; 
      }
    }
    ckFinger.attach(0.2, getIDfinger);
  }
}
void setup() {
  Serial.begin(9600);
  pinMode(BUZZ,OUTPUT); tick();

  finger.begin(57600); 
  if (!finger.verifyPassword())
  {
    lcd_print("t0.txt=\"# Finger Not Found #\"");
    tick(1,1000);
    while(1)
    {
      delay(10000);
    }
  }
  tick();
  ///////////////////////////////////////////////////////////////////
    WiFi.begin(ssid, pass);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
  ///////////////////////////////////////////////////////////////////
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    configF.database_url = DATABASE_URL;
    configF.signer.test_mode = true;
    Firebase.begin(&configF, &auth);
    Firebase.reconnectWiFi(true);
    Firebase.setDoubleDigits(5);
    
  dht1.begin();
  dht2.begin();
  dht3.begin();
  lcd.begin(9600);
  STM.begin(9600);
   
  tick(3,80);
  ticker.detach();

  ckFinger.attach(0.2, getIDfinger);
}

void loop() {
  STM.listen();
  STM.println("CHECK"); delay(20);
  if (STM.available())
  {
    Serial.println("-------------------------");
    String rev = STM.readStringUntil('\n');
    removeNullChars(rev);
    Serial.println(rev);
    if (rev.length()>=7)
    {

      String ss1 = rev.substring(0,2);
      String ss2 = rev.substring(3,5);
      String ss3 = rev.substring(6);
      gas1 = ss1.toInt();
      gas2 = ss2.toInt();
      gas3 = ss3.toInt();
      if (gas1<0) gas1=0;
      if (gas2<0) gas2=0;
      if (gas3<0) gas3=0;
    }
  }
  
  h1 = dht1.readHumidity(); 
  t1 = dht1.readTemperature();
  h2 = dht2.readHumidity();
  t2 = dht2.readTemperature();
  h3 = dht3.readHumidity();
  t3 = dht3.readTemperature();

  char str[50];
  sprintf(str,"%4.1f*C %4.1f%% %2d%%",t1,h1,gas1);
  lcd_print("t0.txt=\"" + String(str) + "\"");
  sprintf(str,"%4.1f*C %4.1f%% %2d%%",t2,h2,gas2);
  lcd_print("t1.txt=\"" + String(str) + "\"");
  sprintf(str,"%4.1f*C %4.1f%% %2d%%",t3,h3,gas3);
  lcd_print("t2.txt=\"" + String(str) + "\"");

  if (Firebase.ready()) 
  {
    sprintf(str,"%4.1f-%4.1f-%02d-%4.1f-%4.1f-%02d-%4.1f-%4.1f-%02d",t1,h1,gas1,t2,h2,gas2,t3,h3,gas3);
    Firebase.RTDB.setString(&fbdo, F("/Sensor"), String(str));
    Firebase.RTDB.getString(&fbdo, "/Control");
    String revs = fbdo.to<String>();
    Serial.println("------------------------------");
    Serial.println(revs);
    Serial.println("------------------------------");
    revs.replace("\\\"","");
//    Serial.println(rev);
    st1 = revs[0]-48;
    st2 = revs[2]-48;
    st3 = revs[4]-48;
    st4 = revs[6]-48;
    st5 = revs[8]-48;
    st6 = revs[10]-48;
  }

  if (gas1>20 || gas2>20 || gas3>20 || t1>36 || t2>36 || t3>36)
  {
    ticker.attach(0.1, blink);
    checkWarn = 1;
    if (gas1>20) Firebase.RTDB.setString(&fbdo, F("/Warning"), "Cảnh_báo_khí_gas_Phòng_Khách!");
    else if (gas2>20) Firebase.RTDB.setString(&fbdo, F("/Warning"), "Cảnh_báo_khí_gas_Phòng_Ngủ!");
    else if (gas3>20) Firebase.RTDB.setString(&fbdo, F("/Warning"), "Cảnh_báo_khí_gas_Phòng_Bếp!");
    else if (t1>36) Firebase.RTDB.setString(&fbdo, F("/Warning"), "Cảnh_báo_nhiệt_độ_cao_Phòng_Khách!");
    else if (t2>36) Firebase.RTDB.setString(&fbdo, F("/Warning"), "Cảnh_báo_nhiệt_độ_cao_Phòng_Ngủ!");
    else if (t3>36) Firebase.RTDB.setString(&fbdo, F("/Warning"), "Cảnh_báo_nhiệt_độ_cao_Phòng_Bếp!");
  }
  else
  {
    ticker.detach();
    digitalWrite(BUZZ,0);
    if (checkWarn==1)
    {
      Firebase.RTDB.setString(&fbdo, F("/Warning"), F("_"));
      checkWarn = 0;
    }
  }
  
  lcd.listen();
  if (lcd.available())
  {
    String rev = lcd.readStringUntil('\n');
    Serial.println(rev);
    if (rev.indexOf("Led1ON")!=-1)  st1=1;
    else if (rev.indexOf("Led1OFF")!=-1) st1=0;
    else if (rev.indexOf("Led2ON")!=-1)  st2=1;
    else if (rev.indexOf("Led2OFF")!=-1) st2=0;
    else if (rev.indexOf("Led3ON")!=-1)  st3=1;
    else if (rev.indexOf("Led3OFF")!=-1) st3=0;
    else if (rev.indexOf("FanON")!=-1)   st4=1;
    else if (rev.indexOf("FanOFF")!=-1)  st4=0;
    else if (rev.indexOf("OpenGATE")!=-1)  st5=1;
    else if (rev.indexOf("CloseGATE")!=-1) st5=0;
    else if (rev.indexOf("OpenDOOR")!=-1)  st6=1;
    else if (rev.indexOf("CloseDOOR")!=-1) st6=0;

    String da = String(st1)+"-"+String(st2)+"-"+String(st3)+"-"+String(st4)+"-"+String(st5)+"-"+String(st6);
    Firebase.RTDB.setString(&fbdo, F("/Control"), da);
  }

  if (ckID > 0) // đúng vân tay theo ID
  {
    Serial.println("Finger ID: "+String(ckID));
    st6 = 1; // mở cửa
    tick(2,80);
    String da = String(st1)+"-"+String(st2)+"-"+String(st3)+"-"+String(st4)+"-"+String(st5)+"-"+String(st6);
    Firebase.RTDB.setString(&fbdo, F("/Control"), da);
    ckID = -2;
    ckFinger.attach(0.2, getIDfinger);
  }

///////////////////////////////////////////////////////////////////
  if (st1 != sp1)
  {
    tick(1,60);
    if (st1==1) {STM.println("Led1ON"); lcd_print("b0.bco=2016");}
    else        {STM.println("Led1OFF"); lcd_print("b0.bco=50712");}
    sp1 = st1;
  }
  if (st2 != sp2)
  {
    tick(1,60);
    if (st2==1) {STM.println("Led2ON"); lcd_print("b1.bco=2016");}
    else        {STM.println("Led2OFF"); lcd_print("b1.bco=50712");}
    sp2 = st2;
  }
  if (st3 != sp3)
  {
    tick(1,60);
    if (st3==1) {STM.println("Led3ON"); lcd_print("b2.bco=2016");}
    else        {STM.println("Led3OFF"); lcd_print("b2.bco=50712");}
    sp3 = st3;
  }
  if (st4 != sp4)
  {
    tick(1,60);
    if (st4==1) {STM.println("FanON"); lcd_print("b3.bco=2016");}
    else        {STM.println("FanOFF"); lcd_print("b3.bco=50712");}
    sp4 = st4;
  }
  if (st5 != sp5)
  {
    tick(1,60);
    if (st5==1) {STM.println("OpenGATE"); lcd_print("b5.bco=2016");}
    else        {STM.println("CloseGATE"); lcd_print("b5.bco=50712");}
    sp5 = st5;
  }
  if (st6 != sp6)
  {
    tick(1,60);
    if (st6==1) {STM.println("OpenDOOR"); lcd_print("b4.bco=2016");}
    else        {STM.println("CloseDOOR"); lcd_print("b4.bco=50712");}
    sp6 = st6;
  }

}

void lcd_print(String ss)
{
  lcd.print(ss);
  lcd.write(0xFF); lcd.write(0xFF); lcd.write(0xFF);
}
void removeNullChars(String &str) {
  while (str.length() > 0 && str[0] == '\0') {
    str.remove(0, 1);
  }
  while (str.length() > 0 && str[str.length() - 1] == '\0') {
    str.remove(str.length() - 1, 1);
  }
}
