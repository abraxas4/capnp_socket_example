#if 0
'''
the IPC mechanism adopted was TCP sockets, 
specifically using QTcpSocket in the C++ QT client and the standard socket library in Python. 
TCP sockets are a core part of the Internet Protocol Suite, 
allowing for reliable, ordered, and error-checked delivery of a stream of bytes 
between applications running on hosts communicating via an IP network. 
This method is widely used for client-server communication where data is exchanged over a network.
'''
#endif
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QTimer>
#include <QTcpSocket>
#include <QDebug>
#include <iostream>
#include "car_data.capnp.h"
#include <capnp/serialize.h>
#include <QThread>

// Class to receive car data over a network using QTcpSocket and Cap'n Proto for serialization
class CarDataReceiver : public QObject {
    Q_OBJECT

public:
    // Constructor: Initializes UI labels, connection timer, and connects signals to slots
    CarDataReceiver(QLabel* speedLabel, QLabel* yawRateLabel)
        : speedLabel(speedLabel), yawRateLabel(yawRateLabel), timer(new QTimer(this)) {
        timer->setInterval(5000); // Attempt to reconnect every 5 seconds if not connected

        // If not connected, attempt to connect to the server at "localhost" on port 12345
        connect(timer, &QTimer::timeout, this, [this]() {
            if (!socket.isOpen()) {
                qDebug() << "Attempting to connect to the server...";
                socket.connectToHost("localhost", 12345);
            }
        });

        // Connect the signal for updating speed and yaw rate to its slot
        connect(this, &CarDataReceiver::updateSpeedAndYawRate, this, &CarDataReceiver::onUpdateSpeedAndYawRate);

        // Setup actions for when connected: stop timer, start reading data
        connect(&socket, &QTcpSocket::connected, this, [this]() {
            qDebug() << "Connected to server!";
            timer->stop(); // Stop attempting to reconnect
            connect(&socket, SIGNAL(readyRead()), this, SLOT(readData())); // Start reading data
        });

        // Restart connection attempts upon socket error
        connect(&socket, &QTcpSocket::errorOccurred, this, [this](QTcpSocket::SocketError socketError) {
            qDebug() << "Socket error occurred:" << socket.errorString();
            timer->start(); // Attempt to reconnect
        });

        timer->start(); // Begin connection attempts
    }

    // Destructor: Cleans up the timer resource
    ~CarDataReceiver() {
        delete timer;
    }

signals:
    // Signal to indicate new speed and yaw rate data is available
    void updateSpeedAndYawRate(float speed, float yawRate);

private slots:
    // Slot to update the UI with new speed and yaw rate
    void onUpdateSpeedAndYawRate(float speed, float yawRate);

    // Slot to handle data reception and processing
    void readData();

private:
    // Handles resetting and reconnecting the socket connection
    void resetConnection() {
        socket.abort(); // Abort current connection
        timer->start(); // Retry connection
        currentMessageSize = 0; // Reset expected message size
        qDebug() << "Connection reset. Attempting to reconnect...";
    }

    // Processes a single message, deserializing and updating UI
    void processMessage(const QByteArray& serializedData) {
        // Align data as per Cap'n Proto requirements to avoid unaligned access
        std::vector<capnp::word> alignedBuffer((serializedData.size() + sizeof(capnp::word) - 1) / sizeof(capnp::word));
        memcpy(alignedBuffer.data(), serializedData.data(), serializedData.size());
        
        // Create a Cap'n Proto reader for the aligned buffer
        kj::ArrayPtr<const capnp::word> words(alignedBuffer.data(), alignedBuffer.size());
        ::capnp::FlatArrayMessageReader reader(words);
        
        // Read the car data and emit a signal to update the UI
        auto carData = reader.getRoot<CarData>();
        emit updateSpeedAndYawRate(carData.getSpeed(), carData.getYawRate());
    }

    QTcpSocket socket; // Socket for network communication
    QLabel* speedLabel; // UI label for displaying speed
    QLabel* yawRateLabel; // UI label for displaying yaw rate
    QTimer* timer; // Timer for attempting reconnections
    const uint32_t MAX_MESSAGE_SIZE = 4096; // Maximum expected message size
    uint32_t currentMessageSize = 0; // Size of the current incoming message
};

// Slot implementation to update UI labels with new speed and yaw rate
void CarDataReceiver::onUpdateSpeedAndYawRate(float speed, float yawRate) {
    // Ensure this slot is executed in the GUI thread
    // This check is crucial for thread safety, as UI updates must occur on the main GUI thread.
    // If this slot is called from a non-GUI thread, we re-invoke the method in the correct thread context.
    if(QThread::currentThread() != this->thread()){
        // Use Qt's meta-object system to safely invoke the method in the GUI thread
        QMetaObject::invokeMethod(this, "onUpdateSpeedAndYawRate", Qt::QueuedConnection,
                                  Q_ARG(float, speed), Q_ARG(float, yawRate));
        return; // Exit the current invocation to wait for the queued call to execute
    }

    // Update the UI with the new speed and yaw rate values
    // This part of the code will only execute if we're already in the GUI thread
    speedLabel->setText(QString("Speed: %1 m/s").arg(speed));
    yawRateLabel->setText(QString("Yaw Rate: %1 degrees/s").arg(yawRate));
}


void CarDataReceiver::readData() {
    // This method reads data from the socket, ensuring messages are complete before processing
    while (socket.bytesAvailable() > 0) {
        if (currentMessageSize == 0) {
            if (socket.bytesAvailable() < sizeof(uint32_t)) {
                qDebug() << "Waiting for more data to determine message size.";
                return;
            }
            QByteArray sizeData = socket.read(sizeof(uint32_t));
            memcpy(&currentMessageSize, sizeData.data(), sizeof(currentMessageSize));
        }

        if (currentMessageSize == 0 || currentMessageSize > MAX_MESSAGE_SIZE) {
            qDebug() << "Invalid message size received. Resetting.";
            resetConnection();
            return;
        }

        if (socket.bytesAvailable() < currentMessageSize) {
            qDebug() << "Waiting for the rest of the message.";
            return;
        }

        qDebug() << "Data received for processing.";
        QByteArray serializedData = socket.read(currentMessageSize);
        try {
            processMessage(serializedData);
        } catch (const std::exception& e) {
            qDebug() << "Exception caught while processing message:" << e.what();
        }

        currentMessageSize = 0;
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QWidget window;

    QVBoxLayout* layout = new QVBoxLayout(&window);
    QLabel speedLabel("Speed: -- m/s");
    QLabel yawRateLabel("Yaw Rate: -- degrees/s");
    layout->addWidget(&speedLabel);
    layout->addWidget(&yawRateLabel);

    CarDataReceiver receiver(&speedLabel, &yawRateLabel);

    window.setLayout(layout);
    window.show();

    return app.exec();
}

#include "main.moc"
