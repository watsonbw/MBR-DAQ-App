#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>

int main(int argc, char *argv[]) {
    // 1. Initialize the Qt Application instance
    QApplication app(argc, argv);

    // 2. Create the main window
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("MBR-DAQ Qt Window");
    mainWindow.resize(400, 300);

    // 3. Create a layout and widgets programmatically
    auto *centralWidget = new QWidget(&mainWindow);
    auto *layout = new QVBoxLayout(centralWidget);

    auto *button = new QPushButton("Click Me!", centralWidget);
    layout->addWidget(button);

    // 4. Connect button click (Signal) to a pop-up dialog (Slot)
    QObject::connect(button, &QPushButton::clicked, [&]() {
        QMessageBox::information(&mainWindow, "Qt Signal", "Hello from Qt6 C++!");
    });

    centralWidget->setLayout(layout);
    mainWindow.setCentralWidget(centralWidget);

    // 5. Display the window
    mainWindow.show();

    // 6. Start the Qt event loop
    return app.exec();
}
