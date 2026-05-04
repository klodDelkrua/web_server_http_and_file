#include "server.h"
#include "request.h"
#include "response.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <iostream>
#include <net/if.h>
#include <memory>

#define MAX_EVENTS 1024
#define BUFFER_SIZE 32768


std::string Server::get_local_ip() {
    std::vector<std::string> possible_ips;
    struct ifaddrs* interfaces = nullptr;

    if (getifaddrs(&interfaces) != 0) {
        perror("getifaddrs");
        return "127.0.0.1";  // Fallback
    }

    for (struct ifaddrs* interface = interfaces;
         interface != nullptr;
         interface = interface->ifa_next) {

        // Vérifie qu'on a une adresse IP valide
        if (interface->ifa_addr == nullptr) continue;

        // On ne veut que les IPv4 (pas IPv6 pour simplifier)
        if (interface->ifa_addr->sa_family != AF_INET) continue;

        // Exclure les interfaces qui ne sont pas actives
        if ((interface->ifa_flags & IFF_UP) == 0) continue;

        // Exclure loopback (127.0.0.1)
        if (interface->ifa_flags & IFF_LOOPBACK) continue;

        // Convertir l'adresse en string
        struct sockaddr_in* addr = (struct sockaddr_in*)interface->ifa_addr;
        std::string ip = inet_ntoa(addr->sin_addr);

        // Stocker toutes les IPs trouvées
        possible_ips.push_back(ip);

        std::cout << "📡 Interface trouvée: " << interface->ifa_name
                  << " → " << ip << std::endl;
         }

    freeifaddrs(interfaces);

    // Choisir la meilleure IP (priorité à WiFi/Ethernet)
    if (possible_ips.empty()) {
        return "127.0.0.1";  // Fallback
    }

    // Priorité : 192.168.x.x > 10.x.x.x > 172.x.x.x
    for (const auto& ip : possible_ips) {
        if (ip.find("192.168.") == 0) return ip;  // WiFi/Ethernet local
    }
    for (const auto& ip : possible_ips) {
        if (ip.find("10.") == 0) return ip;       // Réseau d'entreprise
    }

    // Sinon, retourne la première trouvée
    return possible_ips[0];
}


Server::Server(int port) : port(port), server_fd(-1), epoll_fd(-1) {}

Server::~Server() {
    if (server_fd != -1) close(server_fd);
    if (epoll_fd != -1) close(epoll_fd);
}

void Server::init_sockets() {
    server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    constexpr int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen");
        exit(1);
    }

    std::cout << " Serveur démarré sur http://localhost:" << port << std::endl;
    std::cout << " Pour ton téléphone: http://" << get_local_ip() << ":" << port << std::endl;
}

void Server::init_epoll() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        exit(1);
    }

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
        perror("epoll_ctl");
        exit(1);
    }
}

void Server::handle_event(const struct epoll_event& event) {
    if (event.data.fd == server_fd) {
        // Nouvelle connexion entrante
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        if (const int client_fd = accept(server_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len); client_fd >= 0) {
            fcntl(client_fd, F_SETFL, O_NONBLOCK);
            epoll_event ev{};
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = client_fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
        }
    } else {
        // Données d'un client existant
        handle_client(event.data.fd);
    }
}

void Server::handle_client(const int client_fd) {
    char buffer[BUFFER_SIZE];
    const ssize_t bytes = read(client_fd, buffer, BUFFER_SIZE - 1);

    if (bytes <= 0) {
        close(client_fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
        return;
    }

    buffer[bytes] = '\0';
    const Request req = Request::parse(buffer);

    Response resp;

    // 🆕 Gérer la requête OPTIONS (preflight CORS)
    if (req.method == "OPTIONS") {
        resp.status = 204;  // No Content
        resp.body = "";
        send_response(client_fd, resp);
        close(client_fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
        return;
    }

    if (const RouteHandler* handler = find_handler(req.method, req.path)) {
        resp = (*handler)(req);
    } else if (static_handler && static_handler->is_static_file(req.path)) {
        resp = static_handler->serve_file(req.path);
    }else {
        resp.status = 404;
        resp.body = R"({"error":"Route not found"})";
        resp.headers["Content-Type"] = "application/json";
    }

    send_response(client_fd, resp);

    close(client_fd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
}

void Server::send_response(const int client_fd, const Response& resp) {
    std::string response = "HTTP/1.1 " + std::to_string(resp.status) + " " +
                           get_status_text(resp.status) + "\r\n";

    // Headers CORS (si activés)
    if (cors_enabled) {
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
        response += "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
        response += "Access-Control-Max-Age: 86400\r\n";
    }

    for (const auto& [key, value] : resp.headers) {
        response.append(key);
        response.append(": ");
        response.append(value);
        response.append("\r\n");
    }

    response += "Content-Length: " + std::to_string(resp.body.size()) + "\r\n";
    response += "\r\n";
    response += resp.body;

    send(client_fd, response.c_str(), response.size(), 0);
}

void Server::start() {
    init_sockets();
    init_epoll();

    epoll_event events[MAX_EVENTS];

    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            handle_event(events[i]);
        }
    }
}

// Routes registration
void Server::get(const std::string& path, const RouteHandler& handler) {
    get_routes[path] = handler;
}

void Server::post(const std::string& path, const RouteHandler& handler) {
    post_routes[path] = handler;
}

void Server::put(const std::string& path, const RouteHandler& handler) {
    put_routes[path] = handler;
}

void Server::del(const std::string& path, const RouteHandler& handler) {
    delete_routes[path] = handler;
}

RouteHandler* Server::find_handler(const std::string& method, const std::string& path) {
    if (method == "GET") {
        const auto it = get_routes.find(path);
        if (it != get_routes.end()) return &it->second;
    } else if (method == "POST") {
        const auto it = post_routes.find(path);
        if (it != post_routes.end()) return &it->second;
    } else if (method == "PUT") {
        const auto it = put_routes.find(path);
        if (it != put_routes.end()) return &it->second;
    } else if (method == "DELETE") {
        const auto it = delete_routes.find(path);
        if (it != delete_routes.end()) return &it->second;
    }
    return nullptr;
}

// server.cpp - Version corrigée de set_static_folder

void Server::set_static_folder(const std::string& folder) {
    // Nettoyer le dossier : enlever les slashes au début et à la fin
    std::string clean_folder = folder;

    // Enlever les slashes au début
    while (!clean_folder.empty() && clean_folder[0] == '/') {
        clean_folder.erase(0, 1);
    }

    // Enlever les slashes à la fin
    while (!clean_folder.empty() && clean_folder.back() == '/') {
        clean_folder.pop_back();
    }

    static_handler = std::make_unique<StaticFileHandler>(clean_folder);
    std::cout << "✅ Dossier statique configuré: " << clean_folder << std::endl;
}

void Server::set_cors_enabled(const bool enabled) {
    cors_enabled = enabled;
    std::cout << "🔒 CORS " << (enabled ? "activé" : "désactivé") << std::endl;
}
