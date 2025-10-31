#ifndef WIREGUARDMANAGER_H
#define WIREGUARDMANAGER_H

#include <QObject>
#include <QLibrary>
#include <QString>
#include <QByteArray>
#include <QSettings>
#include <QRegExp>
#include <windows.h>
#include <ws2tcpip.h>  // For sockaddr
#include "wireguard.h"  // Download and include this

class WireGuardManager : public QObject {
    Q_OBJECT
public:
    explicit WireGuardManager(QObject *parent = nullptr);
    ~WireGuardManager();

    bool initialize();
    bool createTunnel(const QString &name, const QByteArray &configData);
    bool startTunnel();
    bool stopTunnel();
    QString getStatus() const;
    QByteArray parseConfigFile(const QString &filePath);

signals:
    void statusChanged(const QString &status);
    void logMessage(const QString &msg);
    void progressChanged(int value);  // New: For UI feedback

private:
    QLibrary m_library;
    WIREGUARD_ADAPTER_HANDLE m_adapter = nullptr;
    QString m_tunnelName;
    // Function pointers (from wireguard.h)
    decltype(&WireGuardCreateAdapter) m_createAdapter = nullptr;
    decltype(&WireGuardOpenAdapter) m_openAdapter = nullptr;
    decltype(&WireGuardCloseAdapter) m_closeAdapter = nullptr;
    decltype(&WireGuardSetAdapterState) m_setState = nullptr;
    decltype(&WireGuardSetConfiguration) m_setConfig = nullptr;
    decltype(&WireGuardGetAdapterState) m_getState = nullptr;  // Optional for status

    bool loadFunctions();
    void log(const QString &msg);
    void cleanup();
};

#endif // WIREGUARDMANAGER_H