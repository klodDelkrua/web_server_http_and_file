//
// Created by lcdelcroix on 04/05/2026.
//
#include "request.h"
#include <sstream>

Request Request::parse(const std::string &raw) {
    Request req;
    std::istringstream stream(raw);
    std::string line;

    //Lire la ligne de requete: "GET /api/test HTTP/1.1
    std::getline(stream, line);
    std::istringstream request_line(line);
    request_line >> req.method >> req.path >> req.version;

    //Lire les headers
    while (std::getline(stream, line) && line != "\r") {
        if (size_t colon = line.find(':'); colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 2);
            if (!value.empty() && value.back() == '\r') value.pop_back();
            req.headers[key] = value;
        }
    }

    //Lire le body (le reste)
    std::string remaining;
    while (std::getline(stream, line)) {
        remaining += line + "\n";
    }
    req.body = remaining;

    return req;
}

std::string Request::get_header(const std::string& name) const {
    const auto it = headers.find(name);
    return it != headers.end() ? it->second : "";
}