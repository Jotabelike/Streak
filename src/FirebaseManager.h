#pragma once
#include <functional>
#include <string>
#include <matjson.hpp>
void updatePlayerDataInFirebase();
void loadPlayerDataFromServer();
void refreshPlayerDataFromServer(std::function<void(bool)> callback);
void completeLevelInFirebase(int stars);
void claimOnServer(const std::string& endpoint, const matjson::Value& payload, std::function<void(bool)> callback);
// Igual que claimOnServer pero entrega tambien el codigo HTTP y el cuerpo de la
// respuesta. El codigo permite distinguir un fallo real (red, 500) de un rechazo
// con significado propio: 409 = ya estaba reclamado/agotado, 403 = todavia no
// cumple los requisitos, 402 = sin saldo. El codigo es 0 cuando la peticion ni
// siquiera llego a salir, y el JSON queda vacio si la respuesta no traia uno.
void claimOnServerEx(const std::string& endpoint, const matjson::Value& payload,
    std::function<void(bool, int, const matjson::Value&)> callback);
void refreshPendingLevelRewardsFromServer(std::function<void(bool)> callback);
