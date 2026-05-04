#include "server.h"
#include "response.h"
#include "request.h"
#include <iostream>
#include "json.h"

using json = nlohmann::json;

// Handler pour la santé du serveur
Response health_handler(const Request& req) {
    Response resp;
    resp.set_json(R"({"status":"OK","service":"Immobilier Marketplace API"})");
    return resp;
}

// Handler pour les annonces test
Response test_listings_handler(const Request& req) {
    const json response = {
        {"success", true},
        {"listings", {
            {{"id", 1}, {"title", "Appartement centre ville"}, {"price", 250000}, {"rooms", 3}},
            {{"id", 2}, {"title", "Maison avec jardin"}, {"price", 450000}, {"rooms", 5}}
        }}
    };

    Response resp;
    resp.set_json(response.dump());
    return resp;
}

// 🆕 Handler pour la page d'accueil (racine /)
Response home_handler(const Request& req) {
    // On peut aussi servir un vrai fichier HTML, mais commençons simple
    std::string html = R"(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Marketplace Immobilier</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: white;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            text-align: center;
        }
        h1 {
            font-size: 3em;
            margin-bottom: 20px;
        }
        .status {
            background: rgba(255,255,255,0.2);
            border-radius: 10px;
            padding: 20px;
            margin: 20px 0;
        }
        .api-link {
            display: inline-block;
            background: white;
            color: #667eea;
            padding: 10px 20px;
            margin: 10px;
            border-radius: 5px;
            text-decoration: none;
            font-weight: bold;
        }
        .api-link:hover {
            transform: scale(1.05);
            transition: 0.3s;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🏠 Marketplace Immobilier</h1>
        <div class="status">
            <h2>✅ Serveur C++ Opérationnel</h2>
            <p>Le serveur web tourne parfaitement !</p>
        </div>

        <h3>📡 Points d'API disponibles :</h3>
        <a href="/api/health" class="api-link">🔍 /api/health</a>
        <a href="/api/listings" class="api-link">📋 /api/listings</a>

        <div class="status">
            <h3>📱 Connecté depuis :</h3>
            <p>Ton téléphone accède bien au serveur !</p>
        </div>
    </div>
</body>
</html>
)";

    Response resp;
    resp.set_html(html);
    return resp;
}

int main() {
    Server server(8080);

    // Routes API
    server.get("/", home_handler);              // 🆕 Page d'accueil
    server.get("/api/health", health_handler);
    server.get("/api/listings", test_listings_handler);

    std::cout << "🚀 Serveur immobilier démarré!" << std::endl;
    std::cout << "📋 Routes disponibles:" << std::endl;
    std::cout << "   GET /" << std::endl;
    std::cout << "   GET /api/health" << std::endl;
    std::cout << "   GET /api/listings" << std::endl;

    server.start();

    return 0;
}