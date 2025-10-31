#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_wgManager(new WireGuardManager(this))
{
    ui->setupUi(this);
    setupUI();
    setupTray();

    // Connect signals
    connect(m_wgManager, &WireGuardManager::statusChanged, this, &MainWindow::onStatusChanged);
    connect(m_wgManager, &WireGuardManager::logMessage, this, &MainWindow::onLogMessage);
    connect(m_wgManager, &WireGuardManager::progressChanged, this, &MainWindow::onProgressChanged);  // New signal
    connect(ui->toggleButton, &QPushButton::clicked, this, &MainWindow::onToggleClicked);
    connect(ui->importButton, &QPushButton::clicked, this, &MainWindow::onImportConfig);

    // Animation for button glow
    m_buttonAnimation = new QPropertyAnimation(ui->toggleButton, "geometry", this);
    m_buttonAnimation->setDuration(200);

    // Initial
    ui->statusLabel->setText("Disconnected");
    ui->progressBar->setVisible(false);
    m_isConnected = false;
    m_isConnecting = false;

    m_wgManager->initialize();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUI() {
    setWindowTitle("WireGuard Client");
    resize(450, 350);  // Slightly wider for clean look
    setMinimumSize(400, 300);

    // Central widget layout (vertical stack for Astrill feel)
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Status label (top, bold)
    ui->statusLabel = new QLabel("Disconnected", central);
    ui->statusLabel->setObjectName("statusLabel");
    ui->statusLabel->setAlignment(Qt::AlignCenter);
    QPalette pal = ui->statusLabel->palette();
    pal.setColor(QPalette::WindowText, Qt::red);  // Red for disconnected
    ui->statusLabel->setPalette(pal);
    mainLayout->addWidget(ui->statusLabel);

    // Spacer
    mainLayout->addSpacerItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // Toggle button (prominent, centered)
    ui->toggleButton = new QPushButton("Connect", central);
    ui->toggleButton->setObjectName("toggleButton");
    ui->toggleButton->setIcon(QIcon(":/icons/connect.png"));  // Green icon
    ui->toggleButton->setIconSize(QSize(20, 20));
    ui->toggleButton->setFixedHeight(50);
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 122, 204, 100));
    shadow->setOffset(0, 2);
    ui->toggleButton->setGraphicsEffect(shadow);
    mainLayout->addWidget(ui->toggleButton, 0, Qt::AlignCenter);

    // Progress bar (hidden initially)
    ui->progressBar = new QProgressBar(central);
    ui->progressBar->setRange(0, 0);  // Indeterminate
    ui->progressBar->setVisible(false);
    mainLayout->addWidget(ui->progressBar, 0, Qt::AlignCenter);

    // Import button (subtle)
    ui->importButton = new QPushButton("Import Config", central);
    mainLayout->addWidget(ui->importButton, 0, Qt::AlignCenter);

    // Spacer
    mainLayout->addSpacerItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // Splitter for logs (collapsible, 70% main / 30% log)
    QSplitter *splitter = new QSplitter(Qt::Vertical, central);
    splitter->setCollapsible(1, true);  // Collapse logs
    mainLayout->addWidget(splitter);

    // Placeholder for future config tree (empty for MVP)
    QTreeWidget *configTree = new QTreeWidget(splitter);
    configTree->setHeaderLabel("Configs");
    configTree->setMinimumHeight(50);

    // Log text edit
    ui->logTextEdit = new QTextEdit(splitter);
    ui->logTextEdit->setReadOnly(true);
    ui->logTextEdit->setMaximumBlockCount(200);
    ui->logTextEdit->setMinimumHeight(100);

    splitter->setSizes(QList<int>() << 200 << 100);  // Initial split

    // Status bar
    statusBar()->showMessage("Ready to import a WireGuard config.");

    // Polish
    central->setLayout(mainLayout);
}

void MainWindow::setupTray() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setToolTip("WireGuard Client");
    m_trayIcon->setIcon(QIcon(":/icons/wg-logo.png"));  // Or default

    m_toggleAction = new QAction("Toggle Connection", this);
    connect(m_toggleAction, &QAction::triggered, this, &MainWindow::toggleConnection);

    QMenu *trayMenu = new QMenu(this);
    trayMenu->addAction(m_toggleAction);
    trayMenu->addSeparator();
    QAction *quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    trayMenu->addAction(quitAction);

    m_trayIcon->setContextMenu(trayMenu);
    m_trayIcon->show();
}

void MainWindow::onToggleClicked() {
    if (m_isConnecting) return;  // Prevent spam
    toggleConnection();
}

void MainWindow::toggleConnection() {
    if (m_isConnected) {
        m_isConnecting = true;
        ui->progressBar->setVisible(true);
        ui->toggleButton->setEnabled(false);
        ui->toggleButton->setText("Disconnecting...");
        if (m_wgManager->stopTunnel()) {
            // Animation: Shrink button slightly
            m_buttonAnimation->setStartValue(ui->toggleButton->geometry());
            m_buttonAnimation->setEndValue(ui->toggleButton->geometry().adjusted(0, 0, -10, -5));
            m_buttonAnimation->start();
            connect(m_buttonAnimation, &QPropertyAnimation::finished, this, &MainWindow::onAnimationFinished);
        }
    } else {
        m_isConnecting = true;
        ui->progressBar->setVisible(true);
        ui->toggleButton->setEnabled(false);
        ui->toggleButton->setText("Connecting...");
        if (m_wgManager->startTunnel()) {
            m_buttonAnimation->setStartValue(ui->toggleButton->geometry());
            m_buttonAnimation->setEndValue(ui->toggleButton->geometry().adjusted(0, 0, 10, 5));
            m_buttonAnimation->start();
            connect(m_buttonAnimation, &QPropertyAnimation::finished, this, &MainWindow::onAnimationFinished);
        }
    }
}

void MainWindow::onAnimationFinished() {
    disconnect(m_buttonAnimation, &QPropertyAnimation::finished, this, &MainWindow::onAnimationFinished);
    m_isConnecting = false;
    ui->progressBar->setVisible(false);
    ui->toggleButton->setEnabled(true);
    updateToggleButton();
}

void MainWindow::updateToggleButton() {
    if (m_isConnected) {
        ui->toggleButton->setText("Disconnect");
        ui->toggleButton->setIcon(QIcon(":/icons/disconnect.png"));
        QPalette pal = ui->toggleButton->palette();
        pal.setColor(QPalette::Button, QColor("#dc3545"));  // Red
        ui->toggleButton->setPalette(pal);
    } else {
        ui->toggleButton->setText("Connect");
        ui->toggleButton->setIcon(QIcon(":/icons/connect.png"));
        QPalette pal = ui->toggleButton->palette();
        pal.setColor(QPalette::Button, QColor("#28a745"));  // Green
        ui->toggleButton->setPalette(pal);
    }
}

void MainWindow::onImportConfig() {
    QString fileName = QFileDialog::getOpenFileName(this, "Import WireGuard Config", "", "Config Files (*.conf)");
    if (fileName.isEmpty()) return;

    // Show progress for import
    ui->progressBar->setRange(0, 0);
    ui->progressBar->setVisible(true);
    statusBar()->showMessage("Importing config...");

    QByteArray configData = m_wgManager->parseConfigFile(fileName);
    ui->progressBar->setVisible(false);
    if (configData.isEmpty()) {
        QMessageBox::warning(this, "Error", "Invalid config file. Check logs.");
        statusBar()->showMessage("Import failed.");
        return;
    }

    QString tunnelName = QFileInfo(fileName).baseName();
    if (m_wgManager->createTunnel(tunnelName, configData)) {
        QMessageBox::information(this, "Success", "Config imported.");
        statusBar()->showMessage("Config ready: " + tunnelName);
        ui->statusLabel->setText("Ready");
        QPalette pal = ui->statusLabel->palette();
        pal.setColor(QPalette::WindowText, QColor("#ffc107"));  // Yellow for ready
        ui->statusLabel->setPalette(pal);
    } else {
        QMessageBox::warning(this, "Error", "Failed to create tunnel.");
    }
}

void MainWindow::onStatusChanged(const QString &status) {
    ui->statusLabel->setText(status);
    m_trayIcon->setToolTip("WireGuard: " + status);
    QPalette pal = ui->statusLabel->palette();
    if (status == "Connected") {
        pal.setColor(QPalette::WindowText, QColor("#28a745"));  // Green
        statusBar()->showMessage("Connected to VPN.");
    } else if (status == "Disconnected") {
        pal.setColor(QPalette::WindowText, QColor("#dc3545"));  // Red
        statusBar()->showMessage("Disconnected.");
    } else if (status == "Ready") {
        pal.setColor(QPalette::WindowText, QColor("#ffc107"));  // Yellow
    } else {
        pal.setColor(QPalette::WindowText, QColor("#ffffff"));  // White default
    }
    ui->statusLabel->setPalette(pal);
}

void MainWindow::onLogMessage(const QString &msg) {
    ui->logTextEdit->append("[" + QTime::currentTime().toString() + "] " + msg);
}

void MainWindow::onProgressChanged(int value) {
    // For future: If determinate progress, set ui->progressBar->setValue(value);
    // Currently indeterminate during connect.
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        if (isVisible()) hide(); else { show(); raise(); activateWindow(); }
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        m_wgManager->stopTunnel();
        event->accept();
    }
}