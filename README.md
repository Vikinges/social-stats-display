Social Stats Display
A multifunctional OLED display for real-time statistics from YouTube, Facebook, Instagram, TikTok, and other social networks. Features web interface, OTA updates, flexible configuration, and easy integration.

Features
Displays subscriber/follower/view counts from multiple social networks
Web interface for configuration and OTA updates
Wi-Fi AP fallback and local web server if API is unavailable
Serial mirroring for debugging
Animated icons and clean UI
Hardware
Board: ESP8266 (NodeMCU, Wemos D1 mini, etc.)
Display: SSD1306 OLED (I2C, 128x64)
Power: 5V via microUSB or external supply
Pinout
Signal	ESP8266 Pin	OLED Pin
SDA	D2 (GPIO4)	SDA
SCL	D1 (GPIO5)	SCL
VCC	3.3V/5V	VCC
GND	GND	GND
Quick Start
Connect the OLED display to the ESP8266 as shown above.
Flash the firmware using PlatformIO or Arduino IDE.
On first boot, connect to the device’s Wi-Fi AP and configure your Wi-Fi and social network settings via the web interface.
The display will show live stats and update automatically.
Social Network Integration
YouTube: API key required
Facebook, Instagram, TikTok: planned (add your API keys and enable in web interface)
License
MIT
