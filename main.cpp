#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));  // Smooth base for dark theme

    // Global Dark Theme Stylesheet (Astrill-like: dark bg, green accents)
    a.setStyleSheet(R"(
        QMainWindow {
            background-color: #1e1e1e;
            color: #ffffff;
        }
        QPushButton {
            background-color: #2d2d2d;
            border: 1px solid #404040;
            border-radius: 5px;
            padding: 8px 16px;
            color: #ffffff;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #404040;
            border-color: #007acc;
        }
        QPushButton:pressed {
            background-color: #1a1a1a;
        }
        QPushButton#toggleButton {
            background-color: #28a745;
            border-color: #28a745;
            min-width: 120px;
        }
        QPushButton#toggleButton:hover {
            background-color: #218838;
        }
        QPushButton#toggleButton:pressed {
            background-color: #1e7e34;
        }
        QPushButton#disconnectButton {
            background-color: #dc3545;
            border-color: #dc3545;
        }
        QPushButton#disconnectButton:hover {
            background-color: #c82333;
        }
        QLabel {
            color: #ffffff;
            font-size: 14px;
        }
        QLabel#statusLabel {
            font-size: 16px;
            font-weight: bold;
            padding: 10px;
            border-radius: 3px;
        }
        QStatusBar {
            background-color: #2d2d2d;
            color: #cccccc;
            font-size: 12px;
        }
        QTextEdit {
            background-color: #1a1a1a;
            border: 1px solid #404040;
            border-radius: 3px;
            color: #e0e0e0;
            font-family: 'Consolas';
            font-size: 11px;
        }
        QProgressBar {
            border: 1px solid #404040;
            border-radius: 5px;
            text-align: center;
            background-color: #2d2d2d;
            color: #ffffff;
        }
        QProgressBar::chunk {
            background-color: #007acc;
            border-radius: 3px;
        }
        QSplitter::handle {
            background-color: #404040;
        }
        QTreeView {
            background-color: #1a1a1a;
            color: #ffffff;
            border: 1px solid #404040;
            selection-background-color: #007acc;
        }
    )");

    MainWindow w;
    w.show();
    return a.exec();
}