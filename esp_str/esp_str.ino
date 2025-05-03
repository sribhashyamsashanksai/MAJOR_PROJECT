  #include <WebServer.h>
  #include <WiFi.h>
  #include <esp32cam.h>
  #include <TinyGPS.h>
  TinyGPS gps;
  float flat=0, flon=0;
  const char* WIFI_SSID = "slabs";
  const char* WIFI_PASS = "12345687";
  
  WebServer server(80);

  static auto loRes = esp32cam::Resolution::find(320, 240);
  static auto midRes = esp32cam::Resolution::find(350, 530);
  static auto hiRes = esp32cam::Resolution::find(800, 600);
  void read_gps()
  {
    bool newData = false;
    unsigned long chars;
    unsigned short sentences, failed;
    for (unsigned long start = millis(); millis() - start < 1000;)
    {
      while (Serial.available())
      {
        char c = Serial.read();
        if (gps.encode(c)) 
          newData = true;
      }
    }

    if (newData)
    {
      
      unsigned long age;
      gps.f_get_position(&flat, &flon, &age);

    }
  }

  void serveJpg()
  {
    auto frame = esp32cam::capture();
    if (frame == nullptr) {
      //Serial.println("CAPTURE FAIL");
      server.send(503, "", "");
      return;
    }
  //Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
  //                static_cast<int>(frame->size()));
  
    server.setContentLength(frame->size());
    server.send(200, "image/jpeg");
    WiFiClient client = server.client();
    frame->writeTo(client);
  }

  void handleRoot() {
  
  if(flat==0)
  read_gps();
  String json = "{\"L\":" +String(flat,6)+","+String(flon,6)+ "}";
    //rcv=0;
    server.send(200, "application/json", json);
  }
  
  void handleJpgLo()
  {
    if (!esp32cam::Camera.changeResolution(loRes)) {
    // Serial.println("SET-LO-RES FAIL");
    }
    serveJpg();
  }

  void handleJpgHi()
  {
    if (!esp32cam::Camera.changeResolution(hiRes)) {
      //Serial.println("SET-HI-RES FAIL");
    }
    serveJpg();
  }
  
  void handleJpgMid()
  {
    if (!esp32cam::Camera.changeResolution(midRes)) {
    // Serial.println("SET-MID-RES FAIL");
    }
    serveJpg();
  }

  void handle_1()
  {
      Serial.print("1"); 
  }
  
  
  void  setup(){
    Serial.begin(9600);
    Serial.println();
    {
      using namespace esp32cam;
      Config cfg;
      cfg.setPins(pins::AiThinker);
      cfg.setResolution(hiRes);
      cfg.setBufferCount(2);
      cfg.setJpeg(80);
  
      bool ok = Camera.begin(cfg);
    //  Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
    }
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
    Serial.print("http://");
    Serial.println(WiFi.localIP());
    //Serial.println("  /cam-lo.jpg");
    //Serial.println("  /cam-hi.jpg");
    //Serial.println("  /cam-mid.jpg");
    server.on("/1", handle_1);
    server.on("/cam-lo.jpg", handleJpgLo);
    server.on("/cam-hi.jpg", handleJpgHi);
    server.on("/cam-mid.jpg", handleJpgMid);
    server.on("/", handleRoot);
    server.begin();
  }
  
  void loop()
  {
    server.handleClient();
  }
