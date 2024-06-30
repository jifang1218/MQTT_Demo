# MQTT_Demo

This project demonstrates how to write an MQTT client application.

**Prerequisite:**

- [C++](https://en.wikipedia.org/wiki/C%2B%2B)
- [Qt](https://en.wikipedia.org/wiki/Qt_(software)) (You can refer to my other blog post, [Qt - A Mini HowTo](https://heather-earth-9da.notion.site/Qt-A-Mini-HowTo-d050eca712954e8c9a268f3a856e7360))
- [MQTT](https://en.wikipedia.org/wiki/MQTT)

**Library versions:**

- Qt: [5.15.12](https://download.qt.io/official_releases/qt/5.15/5.15.12/single/) OpenSource Version
- MQTT: [Eclipse-Paho-Mqtt-c 1.3.9](https://github.com/eclipse/paho.mqtt.c)

**Note:**

1. The project includes references to jsoncpp, but it is not actually used. You can remove the related references in `main.cpp` and also remove the related parts of jsoncpp in `CMakeLists.txt`.
2. For convenience, I have placed a dummy client button on the UI for connection information input. You can remove this button by setting the macro `TEST=0` (this macro is defined in `CMakeLists.txt`).

You can also refer to a video in the `video` directory for an actual usage demonstration.
