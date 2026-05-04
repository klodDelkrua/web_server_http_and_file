//
// Created by lcdelcroix on 03/05/2026.
//

#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

struct Request;
struct Response;

//Callback pour gerer une route: recoit request, retourne response
using RouteHandler = std::function<Response(const Request&)>;

class Server {
    public:
    explicit Server(int port);
    ~Server();

    //Demarrer le serveur (bloquant)
    void start();

    //Enregisstrer une route: GET /api/users -> handler
    void get(const std::string& path, const RouteHandler &handler);
    void post(const std::string& path, const RouteHandler &handler);
    void put(const std::string& path, const RouteHandler &handler);
    void del(const std::string& path, const RouteHandler &handler);

    private:
    int port;
    int server_fd;
    int epoll_fd;

    std::unordered_map<std::string, RouteHandler> get_routes;
    std::unordered_map<std::string, RouteHandler> post_routes;
    std::unordered_map<std::string, RouteHandler> put_routes;
    std::unordered_map<std::string, RouteHandler> delete_routes;

    void init_sockets();
    void init_epoll();
    void handle_event(const struct epoll_event& event);
    void handle_client(int client_fd);
    void send_response(int client_fd, const Response& resp);
    std::string get_local_ip();
    void enable_cors();

    RouteHandler* find_handler(const std::string& method, const std::string& path);
};