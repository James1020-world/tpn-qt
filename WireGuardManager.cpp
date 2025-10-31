#include "WireGuardManager.h"
#include <QDebug>
#include <QUuid>
#include <QFile>
#include <QTextStream>

WireGuardManager::WireGuardManager(QObject *parent) : QObject(parent) {}

WireGuardManager::~WireGuardManager() {
    cleanup();
}

bool WireGuardManager::initialize() {
    m_library.setFileName(QCoreApplication::applicationDirPath() + "/wireguard.dll");
    if (!m_library.load()) {
        log("Failed to load wireguard.dll. Ensure it's in the exe directory.");
        return false;
    }
    if (!loadFunctions()) {
        return false;
    }
    log("WireGuard DLL initialized successfully.");
    return true;
}

bool WireGuardManager::loadFunctions() {
    *(void **)&m_createAdapter = m_library.resolve("WireGuardCreateAdapter");
    *(void **)&m_openAdapter = m_library.resolve("WireGuardOpenAdapter");
    *(void **)&m_closeAdapter = m_library.resolve("WireGuardCloseAdapter");
    *(void **)&m_setState = m_library.resolve("WireGuardSetAdapterState");
    *(void **)&m_setConfig = m_library.resolve("WireGuardSetConfiguration");
    *(void **)&m_getState = m_library.resolve("WireGuardGetAdapterState");

    if (!m_createAdapter || !m_openAdapter || !m_closeAdapter || !m_setState || !m_setConfig || !m_getState) {
        log("Failed to resolve one or more DLL functions.");
        return false;
    }
    return true;
}

bool WireGuardManager::createTunnel(const QString &name, const QByteArray &configData) {
    cleanup();  // Close existing if any

    GUID guid;
    CoCreateGuid(&guid);  // Or use QUuid, but for simplicity

    HRESULT hr = m_createAdapter(reinterpret_cast<const wchar_t*>(name.utf16()), L"WireGuard", &guid, &m_adapter);
    if (FAILED(hr)) {
        log(QString("Failed to create adapter '%1': HRESULT 0x%2").arg(name).arg(hr, 0, 16));
        return false;
    }

    hr = m_setConfig(m_adapter, configData.constData(), static_cast<DWORD>(configData.size()));
    if (FAILED(hr)) {
        log("Failed to apply configuration.");
        cleanup();
        return false;
    }

    m_tunnelName = name;
    emit statusChanged("Ready");
    log(QString("Tunnel '%1' created and configured.").arg(name));
    return true;
}

bool WireGuardManager::startTunnel() {
    if (!m_adapter) {
        log("No adapter created.");
        return false;
    }
    HRESULT hr = m_setState(m_adapter, WireGuardAdapterStateUp);
    if (SUCCEEDED(hr)) {
        emit progressChanged(100);
        emit statusChanged("Connected");
        log("Tunnel started.");
        return true;
    }
    else 
    {
        emit progressChanged(0);  // Fail
    }
    log(QString("Failed to start tunnel: HRESULT 0x%1").arg(hr, 0, 16));
    return false;
}

bool WireGuardManager::stopTunnel() {
    if (!m_adapter) return true;  // Already stopped
    HRESULT hr = m_setState(m_adapter, WireGuardAdapterStateDown);
    if (SUCCEEDED(hr)) {
        emit statusChanged("Disconnected");
        log("Tunnel stopped.");
        return true;
    }
    log(QString("Failed to stop tunnel: HRESULT 0x%1").arg(hr, 0, 16));
    return false;
}

QString WireGuardManager::getStatus() const {
    if (!m_adapter) return "Inactive";
    WIREGUARD_ADAPTER_STATE state;
    HRESULT hr = m_getState(m_adapter, &state);
    if (SUCCEEDED(hr)) {
        switch (state) {
        case WireGuardAdapterStateDown: return "Disconnected";
        case WireGuardAdapterStateUp: return "Connected";
        default: return "Unknown";
        }
    }
    return "Error querying state";
}

QByteArray WireGuardManager::parseConfigFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        log("Failed to open config file.");
        return QByteArray();
    }

    QTextStream in(&file);
    WireGuardConfiguration config = {};
    bool inInterface = false;
    bool inPeer = false;
    WireGuardPeer peer = {};  // Assume single peer for MVP
    config.NumPeers = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;

        if (line == "[Interface]") {
            inInterface = true;
            inPeer = false;
            continue;
        } else if (line == "[Peer]") {
            inInterface = false;
            inPeer = true;
            continue;
        }

        QStringList parts = line.split('=');
        if (parts.size() != 2) continue;
        QString key = parts[0].trimmed().toLower();
        QString value = parts[1].trimmed();

        if (inInterface) {
            if (key == "privatekey") {
                QByteArray keyBytes = QByteArray::fromBase64(value.toUtf8(), QByteArray::Base64UrlEncoding);
                if (keyBytes.size() == 32) {
                    memcpy(config.PrivateKey, keyBytes.constData(), 32);
                } else {
                    log("Invalid PrivateKey length.");
                    return QByteArray();
                }
            } else if (key == "address") {
                // Parse IP/CIDR for Interface.Address (IPv4CIDR array)
                // For MVP, skip or set default
            } else if (key == "listenport") {
                config.ListenPort = value.toUShort();
            }
        } else if (inPeer) {
            if (key == "publickey") {
                QByteArray keyBytes = QByteArray::fromBase64(value.toUtf8(), QByteArray::Base64UrlEncoding);
                if (keyBytes.size() == 32) {
                    memcpy(peer.PublicKey, keyBytes.constData(), 32);
                } else {
                    log("Invalid PublicKey length.");
                    return QByteArray();
                }
            } else if (key == "endpoint") {
                // Parse "host:port" to peer.Endpoint (sockaddr_storage)
                QStringList hostPort = value.split(':');
                if (hostPort.size() >= 2) {
                    QString host = hostPort[0];
                    quint16 port = hostPort[1].toUShort();
                    struct addrinfo hints = {}, *res = nullptr;
                    hints.ai_family = AF_INET;
                    hints.ai_socktype = SOCK_DGRAM;
                    hints.ai_protocol = IPPROTO_UDP;
                    getaddrinfo(host.toUtf8().constData(), nullptr, &hints, &res);
                    if (res) {
                        memcpy(&peer.Endpoint, res->ai_addr, res->ai_addrlen);
                        ((struct sockaddr_in*)&peer.Endpoint)->sin_port = htons(port);
                        freeaddrinfo(res);
                    }
                }
            } else if (key == "allowedips") {
                // Parse CIDR list to peer.AllowedIPs (IPv4CIDR array)
                // For MVP: Assume "0.0.0.0/0" full tunnel
                if (value.contains("0.0.0.0/0")) {
                    peer.AllowedIPs[0].Address = 0;
                    peer.AllowedIPs[0].CIDR = 0;
                    peer.NumAllowedIPs = 1;
                }
                // Extend for multi-CIDR/IPv6
            }
            // PresharedKey, PersistentKeepalive, etc., can be added similarly
        }
    }

    if (memcmp(config.PrivateKey, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 32) == 0) {
        log("No PrivateKey found.");
        return QByteArray();
    }

    // Add peer if valid
    if (memcmp(peer.PublicKey, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 32) != 0) {
        config.Peers[config.NumPeers++] = peer;
    } else {
        log("No valid Peer found.");
        return QByteArray();
    }

    // Serialize to config buffer (flexible array for Peers)
    DWORD configSize = offsetof(WireGuardConfiguration, Peers) + (sizeof(WireGuardPeer) * config.NumPeers);
    QByteArray data(reinterpret_cast<const char*>(&config), configSize);
    log("Config parsed: 1 interface, " + QString::number(config.NumPeers) + " peers.");
    return data;
}

void WireGuardManager::log(const QString &msg) {
    qDebug() << "[WG] " << msg;
    emit logMessage(msg);
}

void WireGuardManager::cleanup() {
    if (m_adapter) {
        m_closeAdapter(m_adapter);
        m_adapter = nullptr;
    }
}