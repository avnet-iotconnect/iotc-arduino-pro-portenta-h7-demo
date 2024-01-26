// Wi-Fi network and password
#define SECRET_SSID ""
#define SECRET_PASS ""

// Fill in the hostname of your IoTConnect broker
#define SECRET_BROKER ""

// These settings MUST match up with those defined in the IoTConnect console
const char* SECRET_CLIENT_ID = "";
const char* SECRET_TOPIC = "";
const char* SECRET_ACK_TOPIC = "";
const char* SECRET_CMD_TOPIC = "";

// Root CA Certificate. This should be the same for everyone.
const char SECRET_ROOTCERT[] PROGMEM = R"( Insert AWS Root CA here)";

// The private key for your device
const char PROGMEM SECRET_PRIKEY[] PROGMEM = R"( Insert Key here)";

// The certificate for your device
const char PROGMEM SECRET_DEVCERT[] PROGMEM = R"( Insert Device Cert here)";


