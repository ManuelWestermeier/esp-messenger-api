#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "wifi-credentials.hpp"

AsyncWebServer server(38787);
AsyncWebSocket ws("/ws");

// WebSocket event handler
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    Serial.printf("Client %u connected\n", client->id());
    client->text("Welcome to the chatroom!");
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

      // Broadcast the message to all clients
      ws.textAll(msg);
    }
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Chatroom</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; }
    #chat { border: 1px solid #ccc; height: 300px; overflow-y: scroll; padding: 10px; }
    #message { width: 80%; padding: 10px; }
    #sendBtn { padding: 10px; }
    .message { margin: 5px 0; }
  </style>
</head>
<body>
  <h2>ESP32 Chatroom</h2>
  <div id="chat"></div>
  <input type="text" id="message" placeholder="Enter message" autocomplete="off" />
  <button id="sendBtn">Send</button>

  <script>
    const ws = new WebSocket('ws://' + location.host + '/ws');
    const chat = document.getElementById('chat');
    const input = document.getElementById('message');
    const sendBtn = document.getElementById('sendBtn');

    ws.onopen = () => {
      appendMessage('System: Connected to chatroom.');
    };

    ws.onmessage = (event) => {
      appendMessage(event.data);
    };

    ws.onclose = () => {
      appendMessage('System: Disconnected from chatroom.');
    };

    sendBtn.onclick = sendMessage;
    input.addEventListener("keyup", function(event) {
      if (event.key === "Enter") {
        sendMessage();
      }
    });

    function appendMessage(msg) {
      const div = document.createElement('div');
      div.textContent = msg;
      div.classList.add('message');
      chat.appendChild(div);
      chat.scrollTop = chat.scrollHeight;
    }

    function sendMessage() {
      const msg = input.value.trim();
      if (msg.length > 0) {
        ws.send(msg);
        input.value = '';
      }
    }
  </script>
</body>
</html>
)rawliteral";

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // Attach WebSocket event handler
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Serve the chat HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });

  // Favicon route
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(204); });

  server.begin();
}

void loop()
{
  // Nothing needed here, async server runs in background
}
