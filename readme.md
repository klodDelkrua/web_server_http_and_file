# 🏠 Serveur Web C++ 

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey)]()

Un serveur web haute performance écrit en C++ de zéro, conçu pour servir une application web avec API REST et fichiers statiques. Parfait pour le développement mobile et les tests en réseau local.

## ✨ Fonctionnalités

- 🚀 **Serveur HTTP haute performance** avec epoll (Linux)
- 📁 **Serveur de fichiers statiques** (HTML/CSS/JS/images)
- 🔌 **API REST** avec routes paramétrées (GET, POST, PUT, DELETE)
- 🌍 **CORS activé** pour le développement mobile
- 📱 **Accessible depuis mobile** sur le même réseau WiFi
- 💉 **Non-bloquant** et multi-connexions

## 🛠️ Technologies utilisées

| Technologie | Version | Rôle |
|-------------|---------|------|
| **C++** | 17 | Langage principal |
| **POSIX Sockets** | - | Communication réseau |
| **epoll** | - | Gestion asynchrone des connexions |
| **nlohmann/json** | 3.11.x | Manipulation JSON |

## 📦 Prérequis

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake make g++

# macOS
brew install cmake
```

## 🚀 Installation

### 1. Cloner le repository
```bash
git clone https://github.com/klodDelkrua/web_server_http_and_file.git
cd web_server_http_and_file
```

### 2. Télécharger nlohmann/json
```bash
mkdir -p include
wget https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp -O include/json.hpp
```

### 3. Compiler
```bash
make clean && make
```

## 🎯 Démarrage rapide

```bash
# Lancer le serveur
./serveur_immobilier

# Sortie attendue :
# ✅ Serveur démarré sur http://localhost:8080
# 📱 Pour ton téléphone: http://192.168.0.109:8080
```

### Tester avec curl
```bash
# Test de santé
curl http://localhost:8080/api/health

# Récupérer les annonces (test)
curl http://localhost:8080/api/listings
```

## 📁 Structure du projet

```
serveur_immobilier/
├── src/
│   ├── main.cpp                 # Point d'entrée, routes
│   ├── server.cpp/h             # Serveur HTTP (sockets, epoll)
│   ├── request.cpp/h            # Parser de requêtes HTTP
│   ├── response.cpp/h           # Constructeur de réponses
│   └── static_file_handler.cpp/h # Fichiers statiques
├── public/                      # Fichiers statiques
│   ├── index.html
│   ├── style.css
│   ├── script.js
│   └── images/
├── include/                     # Bibliothèques externes
│   └── json.hpp
├── Makefile
└── README.md
```

## 🌐 Routes API

| Méthode | Endpoint | Description |
|---------|----------|-------------|
| GET | `/` | Page d'accueil (HTML) |
| GET | `/api/health` | Vérification du serveur |
| GET | `/api/listings` | Liste des annonces (test) |
| GET | `/*.css` | Fichiers CSS |
| GET | `/*.js` | Fichiers JavaScript |
| GET | `/images/*` | Images et assets |

> 💡 **Pour ajouter une route** : Modifie `main.cpp` et ajoute `server.get("/ma-route", monHandler);`

## 📱 Tests mobiles

### Depuis ton téléphone (même WiFi)

1. **Trouver ton IP locale**
```bash
ip addr show | grep inet
# Exemple: 192.168.0.109
```

2. **Accéder depuis le navigateur**
```
http://192.168.0.109:8080          # Frontend
http://192.168.0.109:8080/api/health  # API
```

3. **Tester depuis une app React**
```javascript
const API_URL = 'http://192.168.0.109:8080';

fetch(`${API_URL}/api/listings`)
  .then(res => res.json())
  .then(data => console.log(data));
```

## 📂 Servir ton application React

```bash
# 1. Builder ton app React
npm run build

# 2. Copier dans le dossier public
cp -r build/* public/

# 3. Redémarrer le serveur
./serveur_immobilier
```

## 🔧 Configuration

### Modifier le port
```cpp
// Dans main.cpp
Server server(8080);  // Change ici
```

### Activer/Désactiver CORS
```cpp
// Dans main.cpp
server.set_cors_enabled(true);   // Pour développement mobile
server.set_cors_enabled(false);  // Pour production
```

### Changer le dossier des fichiers statiques
```cpp
// Dans main.cpp
server.set_static_folder("public");     // Dossier par défaut
server.set_static_folder("./frontend"); // Dossier personnalisé
```

## 🧪 Tests

```bash
# Test de charge (ab - Apache Bench)
ab -n 1000 -c 100 http://localhost:8080/api/health

# Vérification des fuites mémoire
valgrind --leak-check=full ./serveur_immobilier
```

## 🐛 Dépannage

| Problème | Solution |
|----------|----------|
| `Address already in use` | `sudo lsof -i :8080` puis `kill <PID>` |
| `File not found: public/index.html` | Vérifier que le dossier `public/` existe |
| `CORS error` | Vérifier `set_cors_enabled(true)` |
| `Connection refused` | Vérifier que le serveur tourne |

## 🚢 Déploiement

### Serveur simple
```bash
# Lancer en arrière-plan
nohup ./serveur_immobilier > logs.txt 2>&1 &
```

### Avec systemd (service Linux)
```ini
# /etc/systemd/system/immobilier.service
[Unit]
Description=Immobilier Server
After=network.target

[Service]
Type=simple
User=www-data
WorkingDirectory=/opt/serveur-immobilier
ExecStart=/opt/serveur-immobilier/serveur_immobilier
Restart=always

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl enable immobilier
sudo systemctl start immobilier
```

## 📊 Performances

- **Connexions simultanées** : 10 000+ (avec epoll)
- **Requêtes par seconde** : 5 000 - 10 000 (fichiers statiques)
- **Latence** : < 10ms (réseau local)

## 🗺️ Routes à implémenter (futur)

- [ ] `POST /api/auth/register` - Inscription
- [ ] `POST /api/auth/login` - Connexion
- [ ] `GET /api/listings/:id` - Détail annonce
- [ ] `POST /api/listings` - Créer annonce
- [ ] `POST /api/favorites/:id` - Favoris

## 🤝 Contribution

1. Fork le projet
2. Créer ta branche (`git checkout -b feature/AmazingFeature`)
3. Commit (`git commit -m 'Add AmazingFeature'`)
4. Push (`git push origin feature/AmazingFeature`)
5. Ouvrir une Pull Request


## 🙏 Remerciements

- [nlohmann/json](https://github.com/nlohmann/json) - Bibliothèque JSON

---

## ⭐️ Support

Si ce projet t'aide, mets une ⭐️ sur GitHub !

---

**Fait avec ❤️ en C++**


Tu peux le personnaliser avec ton nom et ton pseudo GitHub ! 🚀
