//
// Created by lcdelcroix on 04/05/2026.
//

#pragma once
#include <string>
#include <unordered_map>

struct Response {
    int status;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    Response() : status(0) {
        headers["Content-Type"] = "application/json";
    }

    void set_json(const std::string& json) {
        body = json;
        headers["Content-Type"] = "application/json";
    }

    void set_text(const std::string& text) {
        body = text;
        headers["Content-Type"] = "text/plain";
    }

    void set_html(const std::string& html) {
        body = html;
        headers["Content-Type"] = "text/html; charset=utf-8";
    }
};

inline std::string get_status_text(const int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default: return "Unknown";
    }
}