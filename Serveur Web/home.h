#ifndef HOME_H
#define HOME_H

#include <WebServer.h>
#include <String>

// Définition de la structure ConnectedClient
typedef struct {
  String mac;
  String hostname;
  String ip;
  String connectionTime; // Changed back to String for Unix timestamp
  int x_pos;
  int y_pos;
} ConnectedClient;

// Déclarations des fonctions de gestion du serveur web
void handleRoot(WebServer &server, int clientCount, const ConnectedClient *clients);
void handleSetInitialPosition(WebServer &server);
void handleSetPosition(WebServer &server, int clientCount, ConnectedClient *clients, void (*sendPositionCallback)(int, int, int));

// Déclaration des positions initiales
extern const int INITIAL_X;
extern const int INITIAL_Y;

#endif