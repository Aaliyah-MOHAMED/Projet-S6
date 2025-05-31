#include "home.h"
#include <WebServer.h>
#include <String>

// Déclaration des variables pour la position initiale (elles sont const dans main.ino)
extern const int INITIAL_X;
extern const int INITIAL_Y;

// Implémentation de handleRoot avec formulaire pour définir la position initiale
void handleRoot(WebServer &server, int clientCount, const ConnectedClient *clients) {
  String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Master</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 20px;
      background-color: #f4f4f4;
    }
    h1, h2 {
      color: #333;
      text-align: center;
    }
    .client-card {
      background-color: #fff;
      padding: 15px;
      margin-bottom: 10px;
      border-radius: 5px;
      box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
    }
    .client-info {
      margin-bottom: 5px;
    }
    .initial-position-form {
      background-color: #eee;
      padding: 15px;
      border-radius: 5px;
      margin-top: 20px;
      box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
    }
    .position-input {
      margin-bottom: 10px;
    }
    label {
      display: inline-block;
      width: 100px;
      text-align: right;
      margin-right: 10px;
    }
    input[type="number"] {
      padding: 8px;
      border: 1px solid #ccc;
      border-radius: 4px;
      width: 80px;
    }
    button[type="submit"] {
      background-color: #007bff;
      color: white;
      padding: 10px 15px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-size: 16px;
    }
    button[type="submit"]:hover {
      background-color: #0056b3;
    }
  </style>
</head>
<body>
  <h1>ESP32 Maître</h1>
  <p style="text-align:center;">Nombre d'appareils : )=====";

  html += String(clientCount);
  html += R"=====(</p>)=====";

  for (int i = 0; i < clientCount; i++) {
    html += R"=====(
    <div class="client-card">
      <h2>Appareil )=====" + String(i + 1) + R"=====(</h2>
      <div class="client-info"><strong>MAC:</strong> )=====" + clients[i].mac + R"=====(</div>
      <div class="client-info"><strong>IP:</strong> )=====" + clients[i].ip + R"=====(</div>
      <div class="client-info"><strong>Position:</strong> X=)=====" + String(clients[i].x_pos) + R"=====( Y=)=====" + String(clients[i].y_pos) + R"=====(</div>
      <div class="client-info"><strong>Connecté depuis:</strong> )=====" + clients[i].connectionTime + R"=====(</div>
    </div>)=====";
  }

  html += R"=====(
  <div class="initial-position-form">
    <h2>Définir Position Initiale</h2>
    <form method="post" action="/set_position">
      <div class="position-input">
        <label for="x">Position X:</label>
        <input type="number" id="x" name="x" min="0" max="100" value="50" required>
      </div>
      <div class="position-input">
        <label for="y">Position Y:</label>
        <input type="number" id="y" name="y" min="0" max="100" value="50" required>
      </div>
      <button type="submit">Appliquer Position</button>
    </form>
  </div>
</body>
</html>)=====";

  server.send(200, "text/html", html);
}

void handleSetPosition(WebServer &server, int clientCount, ConnectedClient *clients, void (*sendPositionCallback)(int, int, int)) {
  if (server.hasArg("x") && server.hasArg("y")) {
    int x = server.arg("x").toInt();
    int y = server.arg("y").toInt();
    // Envoyer la nouvelle position à tous les clients connectés
    for (int i = 0; i < clientCount; i++) {
      sendPositionCallback(i, x, y);
    }
    server.send(200, "text/plain", "Position mise à jour et envoyée aux clients.");
  } else {
    server.send(400, "text/plain", "Arguments X et Y manquants.");
  }
}