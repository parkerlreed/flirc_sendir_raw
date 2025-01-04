#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include <QWidget>
#include <QWheelEvent>
#include <QMouseEvent>
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

// Custom QWidget class to handle mouse events
class RemoteWidget : public QWidget {
public:
    explicit RemoteWidget(QWidget* parent = nullptr,
                          const std::map<QString, std::pair<std::vector<uint16_t>, std::vector<uint16_t>>>& codes = {})
    : QWidget(parent), irCodes(codes) {}

protected:
    void wheelEvent(QWheelEvent* event) override {
        if (event->angleDelta().y() > 0) {
            triggerAction("Up");
        } else if (event->angleDelta().y() < 0) {
            triggerAction("Down");
        }
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::MiddleButton) {
            triggerAction("Select");
        }
    }

public:
    void buttonClicked(const QString& action) {
        triggerAction(action);
    }

private:
    const std::map<QString, std::pair<std::vector<uint16_t>, std::vector<uint16_t>>>& irCodes;
    std::map<QString, int> globalState;

    void triggerAction(const QString& action) {
        if (irCodes.find(action) != irCodes.end()) {
            globalState[action] = 1 - globalState[action]; // Toggle state
            sendIRCode(globalState[action] == 0 ? irCodes.at(action).first : irCodes.at(action).second, 2300, 0);
        }
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Initialize the device at the start of the program
    if (!initializeDevice()) {
        return -1;
    }

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
        {1818, 1654, 865, 861, 931, 778, 1759, 1703, 902, 806, 900, 817, 1767, 896, 922, 823, 897, 1702, 897, 839, 868}}},
        {"Volume -", {{1778, 835, 947, 1638, 921, 784, 1772, 1684, 918, 787, 1797, 1696, 1823, 837, 912, 830, 911, 1683, 887},
        {1775, 1699, 801, 914, 917, 787, 1730, 1726, 890, 815, 1795, 1697, 1823, 837, 869, 873, 869, 1724, 884}}},
        {"Volume +", {{1773, 841, 918, 1667, 915, 789, 1729, 1727, 889, 815, 1795, 1698, 1821, 838, 869, 873, 868, 873, 869},
        {1789, 1691, 795, 913, 878, 826, 1792, 1664, 878, 827, 1847, 1646, 1872, 787, 931, 811, 930, 811, 930}}},
        {"Record", {{1804, 809, 950, 1635, 924, 781, 1775, 1681, 920, 785, 918, 796, 1823, 1689, 1821, 1690, 1846},
        {1778, 1688, 810, 913, 922, 783, 1773, 1683, 918, 786, 917, 798, 1823, 1689, 1830, 1682, 1845}}},
        {"Pause/Mute", {
            {1783, 830, 927, 1657, 925, 780, 1778, 1677, 923, 781, 1735, 906, 920, 1674, 920, 811, 1760, 1752, 894},
            {1752, 1713, 810, 913, 895, 810, 1703, 1752, 892, 813, 1735, 906, 844, 1750, 845, 888, 1759, 1752, 862}
        }},
        {"Bookmark", {{1816, 798, 956, 1628, 902, 801, 1824, 1632, 927, 777, 1813, 1679, 1867, 1644, 956, 776, 956, 776, 927},
        {1768, 1696, 811, 913, 886, 823, 1761, 1690, 884, 821, 1820, 1673, 1845, 1667, 907, 825, 907, 825, 840}}},
        {"Favorite", {{1730, 883, 918, 1668, 891, 814, 1730, 1727, 889, 816, 888, 826, 1823, 838, 869, 873, 869, 1725, 1847},
        {1776, 1690, 811, 913, 919, 786, 1731, 1727, 896, 809, 889, 825, 1822, 838, 869, 873, 869, 1726, 1846}}},
        {"1", {{1811, 801, 955, 1629, 927, 787, 1770, 1677, 923, 782, 1796, 844, 944, 798, 918, 823, 917, 825, 916, 1678, 889},
        {1810, 1656, 809, 913, 927, 779, 1778, 1677, 923, 782, 1797, 844, 918, 823, 918, 824, 917, 824, 916, 1679, 890}}},
        {"2", {{1772, 840, 919, 1667, 916, 789, 1730, 1727, 890, 816, 1795, 846, 870, 881, 860, 873, 869, 1726, 1846},
        {1768, 1699, 810, 913, 881, 824, 1766, 1691, 840, 865, 1822, 819, 907, 835, 906, 836, 906, 1689, 1847}}},
        {"3", {{1779, 834, 949, 1647, 912, 782, 1776, 1682, 919, 787, 1796, 844, 917, 825, 915, 827, 914, 1681, 915, 818, 887},
        {1775, 1691, 810, 914, 918, 787, 1730, 1727, 915, 790, 1796, 845, 914, 828, 913, 829, 911, 1690, 906, 821, 886}}},
        {"4", {{1808, 805, 953, 1632, 926, 780, 1778, 1678, 923, 782, 1798, 844, 917, 825, 916, 1679, 1844, 815, 913},
        {1810, 1656, 809, 914, 926, 779, 1778, 1678, 923, 783, 1797, 845, 918, 823, 918, 1677, 1820, 840, 914}}},
        {"5", {{1768, 845, 884, 1701, 887, 818, 1767, 1702, 871, 820, 1821, 821, 869, 872, 908, 1686, 1847, 1666, 840},
        {1767, 1699, 810, 914, 839, 865, 1790, 1666, 840, 865, 1822, 819, 905, 836, 905, 1689, 1848, 1664, 877}}},
        {"6", {{1814, 800, 955, 1630, 926, 778, 1786, 1670, 926, 778, 1784, 857, 953, 788, 954, 1640, 954, 779, 1836},
        {1817, 1650, 902, 822, 924, 781, 1813, 1643, 927, 778, 1811, 830, 955, 787, 954, 1640, 955, 778, 1838}}},
        {"7", {
            {1791, 822, 930, 1654, 876, 828, 1817, 1639, 901, 803, 1855, 785, 957, 785, 957, 1637, 956, 776, 959, 773, 901},
            {1730, 1736, 810, 913, 888, 817, 1767, 1690, 884, 820, 1821, 821, 869, 872, 908, 1687, 907, 825, 907, 826, 840}
        }},
        {"8", {{1705, 909, 846, 1740, 864, 841, 1704, 1753, 862, 843, 1768, 873, 845, 1749, 1794, 867, 857, 884, 845},
        {1746, 1720, 856, 867, 890, 815, 1704, 1752, 863, 841, 1768, 873, 845, 1749, 1793, 867, 844, 897, 845}}},
        {"9", {{1783, 831, 920, 1665, 919, 787, 1730, 1727, 890, 814, 1796, 846, 913, 1682, 1822, 838, 869, 1725, 885},
        {1776, 1691, 810, 914, 918, 787, 1730, 1727, 890, 815, 1795, 847, 925, 1669, 1822, 838, 869, 1726, 884}}},
        {"0", {
            {1810, 804, 954, 1631, 927, 778, 1779, 1678, 923, 782, 1798, 844, 918, 824, 917, 825, 916, 826, 915, 827, 914},
            {1809, 1657, 809, 914, 926, 779, 1778, 1696, 906, 783, 1798, 843, 921, 821, 918, 825, 915, 826, 916, 826, 914}
        }},
        {"Jump", {{1791, 824, 930, 1655, 901, 805, 1788, 1670, 899, 806, 899, 815, 898, 835, 897, 836, 924, 809, 869, 864, 1809},
        {1793, 1675, 921, 817, 860, 832, 1790, 1667, 875, 834, 871, 839, 933, 800, 932, 803, 931, 802, 930, 802, 1846}}},
        {"Display", {{1767, 848, 907, 1678, 874, 832, 1791, 1666, 901, 804, 901, 814, 930, 802, 930, 803, 1843, 1670, 1841},
        {1706, 1762, 861, 863, 859, 846, 1704, 1753, 815, 890, 814, 900, 845, 888, 845, 888, 1794, 1719, 1793}}}
    };

    // Create the main window
    RemoteWidget window(nullptr, irCodes);
    QGridLayout* layout = new QGridLayout;

    // Add buttons to the layout
    QPushButton* powerButton = new QPushButton("Power");
    QPushButton* modeButton = new QPushButton("Mode");
    QPushButton* upButton = new QPushButton("Up");
    QPushButton* downButton = new QPushButton("Down");
    QPushButton* leftButton = new QPushButton("Left");
    QPushButton* rightButton = new QPushButton("Right");
    QPushButton* selectButton = new QPushButton("Select");
    QPushButton* voldButton = new QPushButton("Volume -");
    QPushButton* voluButton = new QPushButton("Volume +");
    QPushButton* recButton = new QPushButton("Record");
    QPushButton* pauseButton = new QPushButton("Pause/Mute");
    QPushButton* bookmButton = new QPushButton("Bookmark");
    QPushButton* favButton = new QPushButton("Favorite");
    QPushButton* oneButton = new QPushButton("1");
    QPushButton* twoButton = new QPushButton("2");
    QPushButton* threeButton = new QPushButton("3");
    QPushButton* fourButton = new QPushButton("4");
    QPushButton* fiveButton = new QPushButton("5");
    QPushButton* sixButton = new QPushButton("6");
    QPushButton* sevenButton = new QPushButton("7");
    QPushButton* eightButton = new QPushButton("8");
    QPushButton* nineButton = new QPushButton("9");
    QPushButton* jumpButton = new QPushButton("Jump");
    QPushButton* zeroButton = new QPushButton("0");
    QPushButton* displayButton = new QPushButton("Display");


    // Arrange buttons in the layout
    layout->addWidget(powerButton, 0, 0); // Top-left
    layout->addWidget(modeButton, 0, 2);  // Top-right
    layout->addWidget(upButton, 1, 1);    // Centered
    layout->addWidget(leftButton, 2, 0);  // Bottom-left
    layout->addWidget(selectButton, 2, 1); // Center
    layout->addWidget(rightButton, 2, 2); // Bottom-right
    layout->addWidget(downButton, 3, 1);  // Bottom-center
    layout->addWidget(voldButton, 4, 0);
    layout->addWidget(voluButton, 4, 2);
    layout->addWidget(recButton, 5, 0);
    layout->addWidget(pauseButton, 5, 2);
    layout->addWidget(bookmButton, 6, 0);
    layout->addWidget(favButton, 6, 2);
    layout->addWidget(oneButton, 7, 0);
    layout->addWidget(twoButton, 7, 1);
    layout->addWidget(threeButton, 7, 2);
    layout->addWidget(fourButton, 8, 0);
    layout->addWidget(fiveButton, 8, 1);
    layout->addWidget(sixButton, 8, 2);
    layout->addWidget(sevenButton, 9, 0);
    layout->addWidget(eightButton, 9, 1);
    layout->addWidget(nineButton, 9, 2);
    layout->addWidget(jumpButton, 10, 0);
    layout->addWidget(zeroButton, 10, 1);
    layout->addWidget(displayButton, 10, 2);


    // Connect buttons to actions
    std::map<QPushButton*, QString> buttonMap = {
        {powerButton, "Power"},
        {modeButton, "Mode"},
        {upButton, "Up"},
        {downButton, "Down"},
        {leftButton, "Left"},
        {rightButton, "Right"},
        {selectButton, "Select"},
        {voldButton, "Volume -"},
        {voluButton, "Volume +"},
        {recButton, "Record"},
        {pauseButton, "Pause/Mute"},
        {bookmButton, "Bookmark"},
        {favButton, "Favorite"},
        {oneButton, "1"},
        {twoButton, "2"},
        {threeButton, "3"},
        {fourButton, "4"},
        {fiveButton, "5"},
        {sixButton, "6"},
        {sevenButton, "7"},
        {eightButton, "8"},
        {nineButton, "9"},
        {jumpButton, "Jump"},
        {zeroButton, "0"},
        {displayButton, "Display"}
    };

    for (const auto& [button, label] : buttonMap) {
        QObject::connect(button, &QPushButton::clicked, [&window, label]() {
            window.buttonClicked(label);
        });
    }

    window.setLayout(layout);
    window.setWindowTitle("IR Remote");
    window.show();

    int result = app.exec();

    closeDevice();
    return result;
}
