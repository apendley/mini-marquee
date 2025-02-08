// Only include in main.cpp!

// Don't put any spaces in SSID
#define SSID                "MiniMarquee"

// Must be at least 8 characters (or nullptr for no password )
#define PASSPHRASE          "ingrEss65"

// the IP address the web server
#define LOCAL_IP            IPAddress(192, 168, 4, 1)

// should be fine
#define SUBNET_MASK         IPAddress(255, 255, 255, 0)

// Initial display rotation (0 - 3)
#define INITIAL_ROTATION    2