#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QAction>
#include <QPropertyAnimation>
#include <QSplitter>
#include "WireGuardManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onToggleClicked();
    void onImportConfig();
    void onStatusChanged(const QString &status);
    void onLogMessage(const QString &msg);
    void onProgressChanged(int value);
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onAnimationFinished();

private:
    Ui::MainWindow *ui;
    WireGuardManager *m_wgManager;
    QSystemTrayIcon *m_trayIcon;
    QAction *m_toggleAction;
    QPropertyAnimation *m_buttonAnimation;
    bool m_isConnected = false;
    bool m_isConnecting = false;
    void setupUI();
    void setupTray();
    void toggleConnection();
    void updateToggleButton();
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H