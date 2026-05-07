//
// Created by lcdelcroix on 04/05/2026.
//

#pragma once
#include "response.h"
#include "request.h"
#include <string>

class StaticFileHandler {
private:
    std::string root_dir;
    //Determine le Content-Type en fonction de l'extension
    std::string get_mime_type(const std::string& path);
    //Lit un fichier et retourne son contenu
    std::string read_file(const std::string& filepath);
    //Verifi si le fichier existe et est lisible
    bool file_exists(const std::string& filepath);
    //Nettoie le chemin pour eviter les attaqques (path traversal)
    std::string sanitize_path(const std::string& path);
    //Le cache pour les images
    std::string get_cache_control(const std::string& path);

public:
    explicit StaticFileHandler(const std::string&  root_directory);
    //Verifie si le chemin correspond a un fichier statique
    bool is_static_file(const std::string& path);
    //Sert le fichier demande
    Response serve_file(const std::string& path);
    //Sert la page 404 personnalisee
    Response serve_404();
};