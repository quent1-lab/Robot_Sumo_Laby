#pragma once
#include <ArduinoJson.h>

/// Interface de base : ne gère plus la valeur,
/// juste la description IHM (init/config).
class Widget {
public:
    virtual ~Widget() {}
    /// Remplit l’objet JSON de configuration du widget.
    virtual void toInitJSON(JsonObject& o) = 0;

    /// Réagit à un message WebSocket reçu.
    /// Par défaut, ne fait rien (pour les widgets qui n'en ont pas besoin).
    virtual void handleMessage(JsonObject& /*msg*/) {}
};
