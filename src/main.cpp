#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "wifi-credentials.hpp"

AsyncWebServer server(38787);
AsyncWebSocket ws("/ws");

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    Serial.printf("Client %u connected\n", client->id());
    client->text("Hello Client!");
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    Serial.printf("Client %u disconnected\n", client->id());
  }
  else if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len)
    {
      String msg = "";
      for (size_t i = 0; i < len; i++)
      {
        msg += (char)data[i];
      }
      Serial.printf("Received from %u: %s\n", client->id(), msg.c_str());

      // Broadcast to all clients
      ws.textAll("Echo: " + msg);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
  Serial.println(WiFi.localIP());

  // WebSocket Event Handler
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Route für die Hello World Webseite
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", "<!DOCTYPE html><html><head><title>Hello</title></head><body><h1>Hello World!</h1><p>This is a WebSocket test server.</p></body></html>"); });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              request->send(204); // No Content
            });

  server.begin();
}

void loop()
{
  // Async Webserver läuft im Hintergrund
}
