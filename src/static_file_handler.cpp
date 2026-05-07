//
// Created by lcdelcroix on 04/05/2026.
//
#include "static_file_handler.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <utility>
#include <vector>

namespace  fs = std::filesystem;

StaticFileHandler::StaticFileHandler(const std::string& root_directory)
    : root_dir(root_directory) {
    // Nettoyer le chemin racine
    // Enlever les slashes superflus à la fin
    while (!root_dir.empty() && root_dir.back() == '/') {
        root_dir.pop_back();
    }

    // Vérifier que le dossier existe
    if (!fs::exists(root_dir) || !fs::is_directory(root_dir)) {
        std::cerr << "⚠️  Attention: Le dossier '" << root_dir << "' n'existe pas!" << std::endl;
        // Créer le dossier si nécessaire
        if (fs::create_directories(root_dir)) {
            std::cout << "📁 Dossier créé: " << root_dir << std::endl;
        }
    }

    std::cout << "📁 Dossier statique configuré: " << root_dir << std::endl;
}

std::string StaticFileHandler::sanitize_path(const std::string& path) {
    // Éviter les attaques type "../../etc/passwd"
    std::string clean = path;

    // Remplacer les backslashes par des slashes (Windows)
    std::ranges::replace(clean, '\\', '/');

    // Supprimer les ".." qui tentent de sortir du dossier
    size_t pos;
    while ((pos = clean.find("..")) != std::string::npos) {
        // Si on trouve ".." au début ou après un slash
        if (pos == 0 || clean[pos-1] == '/') {
            // Chercher le slash avant
            if (const size_t prev_slash = clean.rfind('/', pos-1); prev_slash != std::string::npos) {
                clean.erase(prev_slash, pos - prev_slash + 2);
            } else {
                clean.erase(pos, 2);
            }
        } else {
            break;
        }
    }

    // Supprimer les slashes en début de chemin
    while (!clean.empty() && clean[0] == '/') {
        clean.erase(0, 1);
    }

    // Si chemin vide, servir index.html
    if (clean.empty()) {
        return "index.html";
    }

    return clean;
}

std::string StaticFileHandler::get_cache_control(const std::string &path) {
    //Images et assets statiques : cache 1 jour
    if (path.find(".jpg") != std::string::npos ||
        path.find(".png") != std::string::npos ||
        path.find(".svg") != std::string::npos ||
        path.find(".css") != std::string::npos ||
        path.find(".js") != std::string::npos) {
        return "public, max-age=86400"; // Cache 24h
    }

    if (path.find(".html") != std::string::npos) {
        return "no-cache, must-revalidate";
    }

    return "no-cache";
}

bool StaticFileHandler::file_exists(const std::string &filepath) {
    return fs::exists(filepath) && fs::is_regular_file(filepath);
}

std::string StaticFileHandler::read_file(const std::string &filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string StaticFileHandler::get_mime_type(const std::string &path) {
    //Extraire l'extension
    const size_t dot = path.rfind('.');
    if (dot == std::string::npos) {
        return "application/octet-stream";
    }

    std::string ext = path.substr(dot);
    std::ranges::transform(ext, ext.begin(), ::tolower);

    // Map des extensions MIME
    if (ext == ".html" || ext == ".htm") return "text/html; charset=utf-8";
    if (ext == ".css") return "text/css; charset=utf-8";
    if (ext == ".js") return "application/javascript; charset=utf-8";
    if (ext == ".json") return "application/json; charset=utf-8";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".ico") return "image/x-icon";
    if (ext == ".txt") return "text/plain; charset=utf-8";
    if (ext == ".pdf") return "application/pdf";
    if (ext == ".xml") return "application/xml";
    if (ext == ".webp") return "image/webp";
    if (ext == ".woff") return "font/woff";
    if (ext == ".woff2") return "font/woff2";
    if (ext == ".ttf") return "font/ttf";

    return "application/octet-stream";
}

bool StaticFileHandler::is_static_file(const std::string& path) {
    // Ne pas interférer avec l'API (/api/*)
    if (path.find("/api/") == 0) {
        return false;
    }

    // Si le chemin est / ou vide, c'est une page statique
    if (path == "/" || path.empty()) {
        return true;
    }

    // Extensions de fichiers statiques courants
    const std::vector<std::string> static_extensions = {
        ".html", ".htm", ".css", ".js", ".json",
        ".png", ".jpg", ".jpeg", ".gif", ".svg", ".ico",
        ".txt", ".pdf", ".xml", ".webp", ".woff", ".woff2", ".ttf"
    };

    for (const auto& ext : static_extensions) {
        if (path.find(ext) != std::string::npos) {
            return true;
        }
    }

    return false;
}

Response StaticFileHandler::serve_file(const std::string& path) {
    Response resp;

    // Nettoyer le chemin
    const std::string clean_path = sanitize_path(path);

    // Construire le chemin complet (sans double slash)
    std::string full_path = root_dir + "/" + clean_path;

    // Normaliser les slashes
    std::ranges::replace(full_path, '\\', '/');

    // Debug : afficher le chemin cherché
    std::cout << "🔍 Cherche fichier: " << full_path << std::endl;

    // Vérifier que le fichier existe
    if (!file_exists(full_path)) {
        std::cerr << "❌ Fichier non trouvé: " << full_path << std::endl;
        return serve_404();
    }

    // Lire le fichier
    const std::string content = read_file(full_path);
    if (content.empty()) {
        resp.status = 500;
        resp.set_text("Erreur interne: Impossible de lire le fichier");
        return resp;
    }

    // Construire la réponse
    resp.status = 200;
    resp.body = content;
    resp.headers["Content-Type"] = get_mime_type(full_path);
    resp.headers["Cache-Control"] = get_cache_control(full_path);

    std::cout << "✅ Servi: " << clean_path << " (" << content.size() << " bytes)" << std::endl;

    return resp;
}

Response StaticFileHandler::serve_404() {
    Response resp;
    resp.status = 404;

    const std::string html = R"(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>404 - Page non trouvée</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            padding: 50px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            min-height: 100vh;
        }
        h1 { font-size: 6em; margin: 0; }
        p { font-size: 1.5em; }
        a { color: white; text-decoration: underline; }
    </style>
</head>
<body>
    <h1>404</h1>
    <p>😕 Oops ! La page que vous cherchez n'existe pas.</p>
    <p><a href="/">Retour à l'accueil</a></p>
</body>
</html>
)";

    resp.set_html(html);
    return resp;
}