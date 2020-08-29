# CWAScan

A simple scanner for phones that are using the German Corona-Warn-App.
The scanner uses an ESP32 with a GPS module for the client and a Flask server for the backend.

## Client

Copy `CWAScanConfig.h.example` to `CWAScanConfig.h` and adapt the settings.
In your Arduino IDE, add `https://dl.espressif.com/dl/package_esp32_index.json` to Preferences >
Additional Boards Manager URLs and install the ArduinoJSON library.
Then, connect your ESP32 and select `No OTA (2MB APP/2MP SPIFFS)` to increase your flash storage
size.
Finally, compile and upload the sketch.

## Server

In the `cwa_scan` folder, copy `config.py.example` to `config.py` and adapt the settings.
Install the dependencies with `pip install -r requirements.txt`.
Run the app with `flask run` or with `uwsgi -w app:app -s ...` and a web server in production.
