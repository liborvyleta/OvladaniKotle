#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#endif
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Deklarace pro displej SSD1306 připojený k I2C (piny SDA, SCL)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


//Teplotní senzory Dallas pripojene do GPIO 2
#define ONE_WIRE_BUS 2

#define teplotni_Hysterze 0.5
float cilovaTeplota = 22;                               //nastaveni cilove teploty na dafultni hodnotu
String cilovateplotaString = String(cilovaTeplota, 2);  // prevod float na string
String ukazatel = "undef";
String ukazatel2 = "undef";
String ukazatelDisplay1 = "undef";
String ukazatelDisplay2 = "undef";
//temperature helper
float temperatureHelper1;
float temperatureHelper;
//define plus and minus
#define PLUS
#define MINUS

// Nastavíme instanci OneWire pro komunikaci s libovolnými zařízeními OneWire
OneWire oneWire(ONE_WIRE_BUS);

// odkaz na teplotní senzor
DallasTemperature sensors(&oneWire);

//Proměnné pro ukládání hodnot teploty
String temperatureC1 = "";
String temperatureC = "";

// Proměnné časovače pro aktualizaci teplotního senzoru
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

// Replace with your network credentials
const char *ssid = "SSID";
const char *password = "PASSWORD";

//prihlasovaci udaje na webove rozhrani
const char *http_username = "root";
const char *http_password = "root";


const char *PARAM_INPUT_1 = "state";
const int outputRele
 = 13;       //GPIO rele
const int buttonPlus = 16;   //tlacitko plus
const int buttonMinus = 14;  //tlacitko minus

int tempPlus = 0;
int tempMinus = 0;

unsigned long CasZmenyPlus;  // promenna pro ulozeni casu zmeny stavu tlacitka
#define PRODLEVAPLUS 3

unsigned long CasZmenyMinus;  // promenna pro ulozeni casu zmeny stavu tlacitka
#define PRODLEVAMINUS 3

//vytvorene AsyncWebServer na portu 80
AsyncWebServer server(80);

String readDSTemperatureC() {
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == -127.00) {
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  } else {
    Serial.print("Teplota 1: ");
    Serial.println(tempC);
    temperatureHelper = tempC;
  }
  return String(tempC);
}

String readDSTemperatureC1() {
  sensors.requestTemperatures();
  float tempC1 = sensors.getTempCByIndex(1);

  if (tempC1 == -127.00) {
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  } else {
    Serial.print("Teplota 2: ");
    Serial.println(tempC1);
    temperatureHelper1 = tempC1;
  }
  return String(tempC1);
}
//HLAVNI WEBOVA STRANKA
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Libor - Kotelna</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" 
  integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" 
  crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .ds-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
 .button {
  display: inline-block;
  background-color: #008CBA;
  border: none;
  border-radius: 4px;
  color: white;
  padding: 16px 40px;
  text-decoration: none;
  font-size: 30px;
  margin: 2px;
  cursor: pointer;
}
.button2 {
  background-color: #f44336;
}
  </style>
</head>
<body>
<br>
  <button style="background-color: #059e8a; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer;" onclick="logoutButton()">Odhlasit se</button>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ 
    xhr.open("GET", "/update?state=1", true); 
    document.getElementById("state").innerHTML = "ON";  
  }
  else { 
    xhr.open("GET", "/update?state=0", true); 
    document.getElementById("state").innerHTML = "OFF";      
  }
  xhr.send();
}
function logoutButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/logout", true);
  xhr.send();
  setTimeout(function(){ window.open("/logged-out","_self"); }, 1000);
}
</script>
<h2>Libor - Kotelna</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Teplota:</span> 
    <span id="temperaturec">%TEMPERATUREC%</span>
    <sup class="units">&deg;C</sup>
  </p>

    <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Teplota 2:</span> 
    <span id="temperaturec1">%TEMPERATUREC1%</span>
    <sup class="units">&deg;C</sup>
  </p>

   <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Nastavena teplota:</span> 
    <span id="cilovateplotaString">%CILOVATEPLOTASTRING%</span>
    <sup class="units">&deg;C</sup>
  </p>


  <p>
    <i class="fas fa-solid fa-fire" style="color:#059e8a;"></i> 
    <span class="ds-labels">1. mistnost:</span> 
    <span id="ukazatel">%UKAZATEL%</span>
  </p>

  <p>
    <i class="fas fa-solid fa-fire" style="color:#059e8a;"></i> 
    <span class="ds-labels">2. mistnost:</span> 
    <span id="ukazatel2">%UKAZATEL2%</span>
  </p>
 
    <br>
   
  <button class="button button2" onmousedown="toggleCheckbox('minus');" ontouchstart="toggleCheckbox('minus');" onmouseup="toggleCheckbox('minus');" ontouchend="toggleCheckbox('minus');">&#8722</button>
  <button class="button" onmousedown="toggleCheckbox('plus');" ontouchstart="toggleCheckbox('plus');" onmouseup="toggleCheckbox('plus');" ontouchend="toggleCheckbox('plus');">+</button>
  </body>
  <script>
   function toggleCheckbox(x) {
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/" + x, true);
     xhr.send();
   }
</script>
<script>
  setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturec").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturec", true);
  xhttp.send();
  }, 10000) ;

  setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturec1").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturec1", true);
  xhttp.send();
  }, 1000) ;

</script>
<script>
  setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("cilovateplotaString").innerHTML = this.responseText;
  }
  };
  xhttp.open("GET", "/cilovateplotaString", true);
  xhttp.send();
  }, 100) ;
</script>
<script>
  setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ukazatel").innerHTML = this.responseText;
  }
  };
  xhttp.open("GET", "/ukazatel", true);
  xhttp.send();
  }, 100) ;
</script>

<script>
  setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ukazatel2").innerHTML = this.responseText;
  }
  };
  xhttp.open("GET", "/ukazatel2", true);
  xhttp.send();
  }, 100) ;
</script>
</html>
)rawliteral";

//WEBOVA STRANKA PRO ODHLASENI
const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <p>Uzivatel odhlasen <a href="/">Zpet na nastaveni kotelny</a>.</p>
  <p><strong>Poznamka:</strong> Zavrete tuto kartu weboveho prohlizece a dokoncete proces odhlaseni.</p>
</body>
</html>
)rawliteral";

// Nahradíi zastupne symboly hodnotami z DS18B20 a vytvorene v kodu
String processor(const String &var) {
  Serial.println(var);
  if (var == "TEMPERATUREC") {
    return temperatureC;
  }

  if (var == "TEMPERATUREC1") {
    return temperatureC1;
  }

  if (var == "CILOVATEPLOTASTRING") {
    return cilovateplotaString;
  }

  if (var == "UKAZATEL") {
    return ukazatel;
  }

  if (var == "UKAZATEL2") {
    return ukazatel2;
  }

  return String();
}
//SETUP

void setup() {
  //inicializace seriove linky 
  Serial.begin(115200);

  sensors.begin();
  temperatureC = readDSTemperatureC();
  temperatureC1 = readDSTemperatureC1();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  pinMode(outputRele, OUTPUT);
  digitalWrite(outputRele, HIGH);

  //pripojeni k wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  //Vypis IP ardresy zarizeni
  Serial.println(WiFi.localIP());

  //Definuje obsluznou funkci pro GET pozadavky na korenovou cestu weboveho serveru s autentizaci a odesilanim HTML odpovedi s nahrazenim hodnot pomoci funkce processor
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(401);
  });

  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", logout_html, processor);
  });

  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", temperatureC.c_str());
  });
  server.on("/temperaturec1", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", temperatureC1.c_str());
  });

  //plus minus tlacitka
  server.on("/plus", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("plus");
    cilovaTeplota = cilovaTeplota + 0.25;
    Serial.println("nova cilova teplota z webu je:");
    Serial.println(cilovaTeplota);
    cilovateplotaString = String(cilovaTeplota, 2);  // prevod float na string
    request->send(200, "text/plain", "ok");
  });

  server.on("/minus", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("minus");
    cilovaTeplota = cilovaTeplota - 0.25;
    Serial.println("nova cilova teplota z webu je:");
    Serial.println(cilovaTeplota);
    cilovateplotaString = String(cilovaTeplota, 2);  // prevod float na string
    request->send(200, "text/plain", "ok");
  });


  server.on("/cilovateplotaString", HTTP_GET, [](AsyncWebServerRequest *request) {
    String cilovateplotaString = String(cilovaTeplota, 2);  // prevod float na string
    request->send_P(200, "text/plain", cilovateplotaString.c_str());
  });

  server.on("/ukazatel", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (temperatureHelper > cilovaTeplota - teplotni_Hysterze) {
      ukazatel = "netopí";
      ukazatelDisplay1 = "netopi";

    } else if (temperatureHelper < cilovaTeplota + teplotni_Hysterze) {
      ukazatel = "topí";
      ukazatelDisplay1 = "topi";
    }
    request->send_P(200, "text/plain", ukazatel.c_str());
  });

  server.on("/ukazatel2", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (temperatureHelper1 > cilovaTeplota - teplotni_Hysterze) {
      ukazatel2 = "netopí";
      ukazatelDisplay2 = "netopi";

    } else if (temperatureHelper1 < cilovaTeplota + teplotni_Hysterze) {
      ukazatel2 = "topí";
      ukazatelDisplay2 = "topi";
    }
    request->send_P(200, "text/plain", ukazatel2.c_str());
  });

  // Start server
  server.begin();
}
//========================================================================LOOP
void loop() {

  if ((millis() - lastTime) > timerDelay) {
    temperatureC = readDSTemperatureC();
    temperatureC1 = readDSTemperatureC1();
    lastTime = millis();
  }

  if (temperatureHelper > cilovaTeplota - teplotni_Hysterze) {
    digitalWrite(13, HIGH);
  } else if (temperatureHelper < cilovaTeplota + teplotni_Hysterze) {
    digitalWrite(13, LOW);
  }

  if (temperatureHelper1 > cilovaTeplota - teplotni_Hysterze) {
    digitalWrite(13, HIGH);
  } else if (temperatureHelper1 < cilovaTeplota + teplotni_Hysterze) {
    digitalWrite(13, LOW);
  }

  if (digitalRead(buttonPlus) == 0)  // je-li tlacitko stisknuto
  {
    if (tempPlus == 1 && millis() - CasZmenyPlus > PRODLEVAPLUS)  // neni-li nastaven priznak
    {
      cilovaTeplota = cilovaTeplota + 0.5;
      Serial.println("nova cilova teplota z tlacitka je:");
      Serial.println(cilovaTeplota);
      tempPlus = 0;
      digitalWrite(buttonPlus, !digitalRead(buttonPlus));
    }
  } else  // neni-li stisknuto tlacitko
  {
    CasZmenyPlus = millis();  // zapamatuj si posledni cas, kdy bylo nestisknuto
    tempPlus = 1;             // nuluj priznak, tlacitko stisknuto
  }

  if (digitalRead(buttonMinus) == 0)  // je-li tlacitko stisknuto
  {
    if (tempMinus == 1 && millis() - CasZmenyMinus > PRODLEVAMINUS)  // neni-li nastaven priznak
    {
      cilovaTeplota = cilovaTeplota - 0.5;
      Serial.println("nova cilova teplota z tlacitka je:");
      Serial.println(cilovaTeplota);
      tempMinus = 0;
      digitalWrite(buttonMinus, !digitalRead(buttonMinus));
    }
  } else  // neni-li stisknuto tlacitko
  {
    CasZmenyMinus = millis();  // zapamatuj si posledni cas, kdy bylo nestisknuto
    tempMinus = 1;             // nuluj priznak, tlacitko stisknuto
  }

  sensors.requestTemperatures();
  float tempC1 = sensors.getTempCByIndex(0);
  float tempC2 = sensors.getTempCByIndex(1);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Teplota: ");
  display.println(tempC1);
  display.print("Teplota 2: ");
  display.println(tempC2);
  display.print("Nastavena teplota:\n");
  display.print(cilovaTeplota);
  display.print("\n");
  display.print("1.pokoj:");
  display.print(ukazatelDisplay1);
  display.print("\n2.pokoj:");
  display.print(ukazatelDisplay2);
  display.display();
}