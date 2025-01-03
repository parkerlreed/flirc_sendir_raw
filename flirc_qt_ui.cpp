#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include <QWidget>
#include <map>
#include <vector>
#include <stdint.h>
#include <iostream>
#include <QtConcurrent>
#include "flirc.h"

// Global device handle
int device = -1;

// Function to initialize the device
bool initializeDevice() {
    device = fl_open_device(0x20A0, "flirc.tv");
    if (device < 0) {
        std::cerr << "Failed to open Flirc device.\n";
        return false;
    }
    std::cout << "Flirc device initialized successfully.\n";
    return true;
}

// Function to close the device
void closeDevice() {
    if (device >= 0) {
        fl_close_device();
        device = -1;
        std::cout << "Flirc device closed.\n";
    }
}

// Function to send IR code
void sendIRCode(const std::vector<uint16_t>& code, uint16_t frequency, uint8_t repeats) {
    if (device < 0) {
        std::cerr << "Flirc device is not initialized. Cannot send IR code.\n";
        return;
    }

    QtConcurrent::run([code, frequency, repeats]() {
        uint16_t len = code.size();

        // Measure the time taken by fl_transmit_raw
        auto start = std::chrono::high_resolution_clock::now();
        int result = fl_transmit_raw(const_cast<uint16_t*>(code.data()), len, frequency, repeats);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;

        if (result < 0) {
            std::cerr << "Failed to transmit raw IR data.\n";
        } else {
            std::cout << "IR code transmitted successfully.\n";
        }

        // Print the elapsed time
        std::cout << "fl_transmit_raw took " << elapsed.count() << " ms to execute.\n";
    });
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Initialize the device at the start of the program
    if (!initializeDevice()) {
        return -1;
    }

    // Create the main window and layout
    QWidget window;
    QGridLayout* layout = new QGridLayout;

    // Define IR codes
    std::map<QString, std::pair<std::vector<uint16_t>, std::vector<uint16_t>>> irCodes = {
        {"Power", {{1781, 835, 952, 1638, 925, 782, 1736, 1727, 919, 788, 1799, 846, 872, 1727, 871, 863, 1853, 811, 872},
        {1735, 1735, 813, 913, 893, 815, 1772, 1690, 886, 821, 1825, 821, 872, 1726, 874, 863, 1852, 812, 908}}},
        {"Mode",  {{1799, 820, 935, 1655, 906, 801, 1796, 1666, 907, 801, 908, 808, 1850, 1668, 1850, 814, 932, 812, 933},
        {1770, 1703, 842, 884, 904, 804, 1763, 1699, 875, 832, 874, 843, 1792, 1726, 1791, 873, 901, 844, 900}}},
        {"Up",    {{1736, 882, 921, 1668, 894, 814, 1736, 1727, 891, 817, 889, 828, 1826, 838, 873, 872, 872, 873, 873, 872, 873},
        {1772, 1700, 792, 935, 844, 863, 1795, 1668, 882, 826, 845, 872, 1853, 811, 908, 837, 908, 836, 909, 837, 908}}},
        {"Down",  {{1774, 833, 954, 1636, 927, 782, 1778, 1684, 923, 785, 920, 797, 1828, 837, 872, 873, 872, 873, 872, 1742, 829},
        {1783, 1689, 791, 936, 922, 784, 1736, 1727, 893, 815, 892, 825, 1826, 838, 873, 871, 873, 872, 874, 1725, 851}}},
        {"Left",  {{1714, 908, 847, 1743, 814, 894, 1705, 1759, 820, 888, 812, 905, 817, 919, 1761, 904, 848, 897, 849, 1750, 814},
        {1707, 1767, 843, 884, 819, 889, 1680, 1783, 813, 895, 813, 904, 848, 888, 1736, 928, 849, 897, 840, 1760, 820}}},
        {"Right", {{1735, 885, 845, 1744, 840, 869, 1707, 1755, 816, 892, 792, 925, 844, 891, 1761, 903, 820, 925, 841, 904, 841},
        {1708, 1766, 795, 932, 813, 894, 1705, 1755, 816, 895, 813, 904, 845, 890, 1736, 929, 841, 903, 868, 877, 848}}},
        {"Select",{{1791, 842, 945, 1631, 931, 777, 1790, 1673, 931, 777, 931, 786, 1842, 823, 957, 788, 956, 1643, 957, 779, 930},
        {1818, 1654, 865, 861, 931, 778, 1759, 1703, 902, 806, 900, 817, 1767, 896, 922, 823, 897, 1702, 897, 839, 868}}}
    };

    // Create buttons
    QPushButton* powerButton = new QPushButton("Power");
    QPushButton* modeButton = new QPushButton("Mode");
    QPushButton* upButton = new QPushButton("Up");
    QPushButton* downButton = new QPushButton("Down");
    QPushButton* leftButton = new QPushButton("Left");
    QPushButton* rightButton = new QPushButton("Right");
    QPushButton* selectButton = new QPushButton("Select");

    // Add buttons to layout
    layout->addWidget(powerButton, 0, 0); // Top-left
    layout->addWidget(modeButton, 0, 2);  // Top-right
    layout->addWidget(upButton, 1, 1);    // Centered
    layout->addWidget(leftButton, 2, 0);  // Bottom-left
    layout->addWidget(selectButton, 2, 1); // Center
    layout->addWidget(rightButton, 2, 2); // Bottom-right
    layout->addWidget(downButton, 3, 1);  // Bottom-center

    // Connect buttons to IR codes
    std::map<QPushButton*, QString> buttonMap = {
        {powerButton, "Power"},
        {modeButton, "Mode"},
        {upButton, "Up"},
        {downButton, "Down"},
        {leftButton, "Left"},
        {rightButton, "Right"},
        {selectButton, "Select"}
    };

    for (const auto& [button, label] : buttonMap) {
        QObject::connect(button, &QPushButton::clicked, [=]() {
            static std::map<QString, int> globalState;
            globalState[label] = 1 - globalState[label]; // Toggle state
            sendIRCode(globalState[label] == 0 ? irCodes.at(label).first : irCodes.at(label).second, 2300, 3);
        });
    }

    // Set layout and show the window
    window.setLayout(layout);
    window.setWindowTitle("IR Remote");
    window.show();

    // Execute the app
    int result = app.exec();

    // Close the device
    closeDevice();
    return result;
}
