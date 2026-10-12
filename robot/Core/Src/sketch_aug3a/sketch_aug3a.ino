#include "OV7670.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>
#include <Credentials.h>
#include <ESP32Servo.h>
#include "BMP.h"

Servo x; //d26
Servo y; //d25
const int servoMin = 1000; //servo min and max values
const int servoMax = 2000;
ESP32PWM pwm;
int xPos = 90;
int yPos = 90;
int servoStep = 10;

const int SIOD = 21;
const int SIOC = 22;

const int VSYNC = 34;
const int HREF = 35;

const int XCLK = 32;
const int PCLK = 33;

const int D0 = 27;
const int D1 = 17;
const int D2 = 16;
const int D3 = 15;
const int D4 = 14;
const int D5 = 13;
const int D6 = 12;
const int D7 = 4;

const int TFT_DC = 2;
const int TFT_CS = 5;

const char* ssid = SSID;
const char* pswd = PSWD;
const char campswd[60] = CAM_PSWD;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, 0);
OV7670 *camera;

WiFiMulti wifiMulti;
WiFiServer server(80);

unsigned char bmpHeader[BMP::headerSize];

bool authenticated = false;

bool hasValidSession(String request)
{
  return request.indexOf("Cookie: camera_session=authenticated") >= 0;
}


void sendLoginPage(WiFiClient &client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  client.print(
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"

    "<style>"
    "body{"
      "margin:0;"
      "background:#222;"
      "color:white;"
      "font-family:Arial;"
      "text-align:center;"
    "}"

    "#login{"
      "margin-top:80px;"
    "}"

    "input{"
      "font-size:18px;"
      "padding:8px;"
      "margin:5px;"
    "}"

    "#camera{"
      "display:none;"
      "max-width:100%;"
      "height:auto;"
    "}"

    "#display{"
      "margin:10px;"
      "font-size:20px;"
    "}"

    "</style>"

    "<script>"
    "function login(){"

      "var password=document.getElementById('password').value;"

      "var xhr=new XMLHttpRequest();"

      "xhr.open('POST','/login',true);"

      "xhr.setRequestHeader('Content-Type','application/x-www-form-urlencoded');"

      "xhr.onreadystatechange=function(){"

        "if(xhr.readyState===4){"

          "if(xhr.status===200){"

            "document.getElementById('display').innerHTML='Accepted';"

            "document.getElementById('login').style.display='none';"

            "document.getElementById('cameraContainer').style.display='block';"

            "document.getElementById('camera').style.display='block';"

            "startCamera();"

          "}else{"

            "document.getElementById('display').innerHTML='Denied';"

          "}"

        "}"

      "};"

      "xhr.send('password='+encodeURIComponent(password));"

    "}"

    "function startCamera(){"

      "var img=document.getElementById('camera');"

      "img.src='/camera?'+Date.now();"

    "}"

    "function nextFrame(){"

      "setTimeout(startCamera,10);"

    "}"

    "function moveCamera(direction){"

      "var xhr=new XMLHttpRequest();"

      "xhr.open('POST','/camera/'+direction,true);"

      "xhr.onreadystatechange=function(){"

        "if(xhr.readyState===4){"

          "if(xhr.status===200){"

            "startCamera();"

          "}"

        "}"

      "};"

      "xhr.send();"

    "}"

    "</script>"

    "</head>"

    "<body>"

    "<div id='login'>"

      "<h2>Camera login</h2>"

      "<input "
        "type='password' "
        "id='password' "
        "placeholder='Password'>"

      "<br>"

      "<input "
        "type='button' "
        "value='OK' "
        "onclick='login()'>"

      "<div id='display'></div>"

    "</div>"

    "<div "
      "id='cameraContainer' "
      "style='display:none;'>"

      "<table>"

        "<tr>"

          "<td rowspan='3'>"

            "<img "
              "id='camera' "
              "onload='nextFrame()' "
              "onerror='nextFrame()'>"

          "</td>"

          "<td>"

            "<button "
              "type='button' "
              "onclick=\"moveCamera('up')\">"
              "Haut"
            "</button>"

          "</td>"

        "</tr>"

        "<tr>"

          "<td>"

            "<button "
              "type='button' "
              "onclick=\"moveCamera('left')\">"
              "Gauche"
            "</button>"

            "<button "
              "type='button' "
              "onclick=\"moveCamera('right')\">"
              "Droite"
            "</button>"

          "</td>"

        "</tr>"

        "<tr>"

          "<td>"

            "<button "
              "type='button' "
              "onclick=\"moveCamera('down')\">"
              "Bas"
            "</button>"

          "</td>"

        "</tr>"

      "</table>"

    "</div>"

    "</body>"
    "</html>"
  );

  client.println();
}

void handleLogin(WiFiClient &client, String request)
{
  String password = "";

  int passwordIndex = request.indexOf("password=");

  if (passwordIndex >= 0)
  {
    password = request.substring(passwordIndex + 9);

    int endIndex = password.indexOf('\r');

    if (endIndex >= 0)
      password = password.substring(0, endIndex);

    endIndex = password.indexOf('\n');

    if (endIndex >= 0)
      password = password.substring(0, endIndex);
  }

  if (password == campswd)
  {
    authenticated = true;

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Set-Cookie: camera_session=authenticated; Path=/");
    client.println("Connection: close");
    client.println();
    client.println("OK");
  }
  else
  {
    authenticated = false;

    client.println("HTTP/1.1 403 Forbidden");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("DENIED");
  }
}


void sendCamera(WiFiClient &client)
{
  if (!authenticated)
  {
    client.println("HTTP/1.1 403 Forbidden");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("Access denied");

    return;
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: image/bmp");
  client.println("Cache-Control: no-cache, no-store, must-revalidate");
  client.println("Pragma: no-cache");
  client.println("Expires: 0");
  client.println("Connection: close");
  client.println();

  client.write(
    bmpHeader,
    BMP::headerSize
  );

  client.write(
    camera->frame,
    camera->xres * camera->yres * 2
  );
}


void sendCommandResponse(WiFiClient &client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Cache-Control: no-cache");
  client.println("Connection: close");
  client.println();
  client.println("OK");
}


void serve()
{
  WiFiClient client = server.accept();

  if (!client)
    return;

  unsigned long timeout = millis();

  while (!client.available() && millis() - timeout < 1000)
  {
    delay(1);
  }

  if (!client.available())
  {
    client.stop();
    return;
  }

  String request = "";

  timeout = millis();

  while (client.connected() && millis() - timeout < 1000)
  {
    while (client.available())
    {
      char c = client.read();

      request += c;

      timeout = millis();

      if (request.indexOf("\r\n\r\n") >= 0)
      {
        break;
      }
    }

    if (request.indexOf("\r\n\r\n") >= 0)
      break;
  }

  if (request.startsWith("GET / "))
  {
    sendLoginPage(client);
  }

  else if (request.startsWith("POST /login"))
  {
    delay(10);

    while (client.available())
    {
      char c = client.read();
      request += c;
    }

    handleLogin(client, request);
  }

  else if (request.startsWith("GET /camera"))
  {
    sendCamera(client);
  }

  else if (request.startsWith("GET /logout"))
  {
    authenticated = false;

    client.println("HTTP/1.1 302 Found");
    client.println("Location: /");
    client.println("Set-Cookie: camera_session=; Max-Age=0; Path=/");
    client.println("Connection: close");
    client.println();
  }

  else if (request.startsWith("POST /camera/up"))
  {
    if (authenticated)
    {
      yPos += servoStep;

      if (yPos > 180)
        yPos = 180;

      y.write(yPos);
    }

    sendCommandResponse(client);
  }

  else if (request.startsWith("POST /camera/down"))
  {
    if (authenticated)
    {
      yPos -= servoStep;

      if (yPos < 0)
        yPos = 0;

      y.write(yPos);
    }

    sendCommandResponse(client);
  }

  else if (request.startsWith("POST /camera/right"))
  {
    if (authenticated)
    {
      xPos += servoStep;

      if (xPos > 180)
        xPos = 180;

      x.write(xPos);
    }

    sendCommandResponse(client);
  }

  else if (request.startsWith("POST /camera/left"))
  {
    if (authenticated)
    {
      xPos -= servoStep;

      if (xPos < 0)
        xPos = 0;

      x.write(xPos);
    }

    sendCommandResponse(client);
  }

  else
  {
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("404 Not Found");
  }

  client.stop();
}


void setup()
{
  Serial.begin(115200);

  ESP32PWM::allocateTimer(0);
  //ESP32PWM::allocateTimer(1);
  //ESP32PWM::allocateTimer(2);
  //ESP32PWM::allocateTimer(3);

  x.setPeriodHertz(50);
  y.setPeriodHertz(50);

  int xChannel = x.attach(26, 1000, 2000);
  int yChannel = y.attach(25, 1000, 2000);

  Serial.print("X channel: ");
  Serial.println(xChannel);

  Serial.print("Y channel: ");
  Serial.println(yChannel);

  Serial.print("X attached: ");
  Serial.println(x.attached());

  Serial.print("Y attached: ");
  Serial.println(y.attached());

  /*
  for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
    x.write(posDegrees);
    Serial.println(posDegrees);
    delay(1);
  }

  for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
    x.write(posDegrees);
    Serial.println(posDegrees);
    delay(1);
  }*/

  wifiMulti.addAP(ssid, pswd);

  Serial.println("Connecting Wifi...");

  uint8_t wifi_run = wifiMulti.run();

  if (wifi_run == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address:");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("WiFi problem");
    Serial.println(wifi_run, DEC);
  }

  camera = new OV7670(
    OV7670::Mode::QQVGA_RGB565,
    SIOD,
    SIOC,
    VSYNC,
    HREF,
    XCLK,
    PCLK,
    D0,
    D1,
    D2,
    D3,
    D4,
    D5,
    D6,
    D7
  );

  BMP::construct16BitHeader(
    bmpHeader,
    camera->xres,
    camera->yres
  );

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(0);

  server.begin();

  Serial.println("HTTP server started");
}


void displayY8(
  unsigned char * frame,
  int xres,
  int yres
)
{
  tft.setAddrWindow(
    0,
    0,
    yres - 1,
    xres - 1
  );

  int i = 0;

  for (int x = 0; x < xres; x++)
  {
    for (int y = 0; y < yres; y++)
    {
      i = y * xres + x;

      unsigned char c = frame[i];

      unsigned short r = c >> 3;
      unsigned short g = c >> 2;
      unsigned short b = c >> 3;

      tft.pushColor(
        r << 11 |
        g << 5 |
        b
      );
    }
  }
}


void displayRGB565(
  unsigned char * frame,
  int xres,
  int yres
){
  tft.setAddrWindow(
    0,
    0,
    yres - 1,
    xres - 1
  );

  int i = 0;

  for (int x = 0; x < xres; x++)
  {
    for (int y = 0; y < yres; y++)
    {
      i = (y * xres + x) << 1;

      tft.pushColor(
        frame[i] |
        (frame[i + 1] << 8)
      );
    }
  }
}

void loop()
{
  camera->oneFrame();

  serve();

  displayRGB565(
    camera->frame,
    camera->xres,
    camera->yres
  );
}
