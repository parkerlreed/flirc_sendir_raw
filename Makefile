# Compiler and flags
CC = g++
CFLAGS = -g -std=c++17 -I /home/parker/build/sdk/libs/include/flirc -I /home/parker/build/sdk/cli/include -I /usr/include/qt -I /usr/include/qt/QtCore -I /usr/include/qt/QtGui -I /usr/include/qt/QtWidgets -I /usr/include/qt/QtConcurrent
LDFLAGS = /home/parker/build/sdk/libs/Linux_x86_64/libflirc.a -lusb-1.0 -lhidapi-hidraw -lQt5Core -lQt5Gui -lQt5Widgets

# Target
TARGET = flirc_qt_ui

# Sources
SRCS = flirc_qt_ui.cpp

# Build rules
all: $(TARGET)

$(TARGET): $(SRCS)
        $(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
        rm -f $(TARGET) *.o
