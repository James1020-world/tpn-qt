# 🛡️ TPN Client for Windows
> **Tao Private Network (TPN)** — Secure, fast, and user-friendly VPN client for Windows  
> Built with **Qt 5.15.2** and **Visual Studio 2019**

---

## 📘 Overview

**TPN Client** is the official **Windows desktop application** that connects end users to the **Tao Private Network (TPN)** — a privacy-focused VPN platform designed to provide secure, high-speed, and censorship-resistant connectivity.

This client allows users to:
- Connect and disconnect from TPN servers with one click  
- Manage multiple VPN profiles and regions  
- Automatically configure Wintun or TAP adapters  
- View live connection stats and session logs  
- Securely handle authentication keys and certificates  

> This application is for **end users** — not for miners, validators, or worker nodes.

---

## 🧩 Key Features

| Feature | Description |
|----------|-------------|
| 🔐 **Secure Tunnel** | Encrypted TLS or DTLS tunnel via the TPN protocol |
| ⚙️ **Automatic Driver Setup** | Integrates with **Wintun** or **TAP-Windows** |
| 🌍 **Multi-Server Profiles** | Connect to various locations and routing modes |
| 🧠 **Smart Routing** | Selective routing and DNS leak protection |
| 💡 **System Tray Integration** | Quick connect/disconnect and status indicator |
| 🪶 **Lightweight** | Optimized Qt C++ core, minimal dependencies |
| 🧾 **Logs & Diagnostics** | Real-time and persistent logging for debugging |

---

## 🖥️ Tech Stack

| Component | Version / Tool |
|------------|----------------|
| **Qt** | 5.15.2 (MSVC2019 64-bit) |
| **Compiler** | Visual Studio 2019 |
| **Build System** | CMake 3.15+ or qmake |
| **Network Driver** | Wintun (recommended) or TAP-Windows |
| **Crypto Library** | OpenSSL 1.1+ |
| **Target OS** | Windows 10 / 11 (x64) |

---

## ⚙️ Prerequisites

Before building or running, make sure you have:

- **Windows 10 / 11 64-bit**
- **Visual Studio 2019** (with “Desktop development with C++” workload)
- **Qt 5.15.2 for MSVC2019 64-bit**
- **CMake 3.15+** *(recommended)*
- **OpenSSL (Win64 build)** *(for TLS)*  
- **Wintun or TAP-Windows driver**

Optional but useful:
- **Qt Creator IDE**
- **Wireshark** (for debugging)
- **NSIS or Inno Setup** (for packaging)
- **signtool.exe** (for code signing)

---
