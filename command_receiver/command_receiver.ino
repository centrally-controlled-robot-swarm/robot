#include <WiFi.h>
#include <WiFiUdp.h>


////////////
// GLOBAL //
////////////

// Hotspot credentials
const char* ssid = "blinky";
const char* password = "swarm_net";

// Manually set IP address
IPAddress local_IP(10, 42, 0, 30);   // Choose something outside DHCP range (e.g., .50)
IPAddress gateway(10, 42, 0, 1);     // Usually the hotspot IP
IPAddress subnet(255, 255, 255, 0);      // Standard subnet mask

// ---- UDP settings ----
WiFiUDP udp;
const unsigned int localPort = 4210;  // Port to listen on

// Input messages will be split into right and left angular velocities
float r_angular_vel, l_angular_vel;


///////////
// SETUP //
///////////

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Configuring static IP...");
    if (!WiFi.config(local_IP, gateway, subnet)) {
        Serial.println("Failed to configure static IP");
    }

    // Connect to WiFi
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());

    // Start UDP
    udp.begin(localPort);
    Serial.print("Listening on UDP port ");
    Serial.println(localPort);
}


////////////////
// MAIN LOOP //
///////////////

void loop() {
    char incomingPacket[255];  // buffer for incoming packets
    int packetSize = udp.parsePacket();
    if (packetSize) {
        // read the packet into buffer
        int len = udp.read(incomingPacket, 255);
        if (len > 0) incomingPacket[len] = '\0';  // null-terminate the string
    
        // I temporarilly commented this out so that I can just test with strings of any format
        // Parse the input string, splitting it by the comma delimiter
        // char *token = strtok(incomingPacket, ",");
        // if (token != NULL) l_angular_vel = atof(token);
        
        // token = strtok(NULL, ",");
        // if (token != NULL) r_angular_vel = atof(token);
        
        Serial.print("Received packet from ");
        Serial.print(udp.remoteIP());
        Serial.print(":");
        Serial.println(udp.remotePort());
        // Serial.printf("l_angular_vel: %.2f, r_angular_vel: %.2f\n", l_angular_vel, r_angular_vel);
        Serial.println(incomingPacket);

        // Echo back the same message to sender
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        udp.print(incomingPacket);
        udp.endPacket();
    }

    // TODO: can lower delay for faster responses
    delay(10); // small delay to prevent spamming Serial
}
