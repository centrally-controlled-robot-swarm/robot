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

// Motor Control
bool halted = false;
    //LEFT MOTOR
int motor1Pin1 = 12;
int motor1Pin2 = 14;
int enable1Pin = 13;
    //RIGHT MOTOR
int motor2Pin1 = 27;
int motor2Pin2 = 26;
int enable2Pin = 25;

// PWM presets
const int freq = 30000;
const int resolution = 8;
const int pwmChannelLeft = 0;
const int pwmChannelRight = 1;

///////////
// SETUP //
///////////

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Motor pins
    pinMode(motor1Pin1, OUTPUT);
    pinMode(motor1Pin2, OUTPUT);
    pinMode(enable1Pin, OUTPUT);

    pinMode(motor2Pin1, OUTPUT);
    pinMode(motor2Pin2, OUTPUT);
    pinMode(enable2Pin, OUTPUT);

    // Configure PWM channels
    ledcAttachChannel(enable1Pin, freq, resolution, pwmChannelLeft);
    ledcAttachChannel(enable2Pin, freq, resolution, pwmChannelRight);

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

//////////////////////
// HELPER FUNCTIONS //
//////////////////////

void startMotors(int leftPWM, int rightPWM) {
    //enables motors to move forward
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, HIGH);
    digitalWrite(motor2Pin2, LOW);
    //starts motion
    ledcWrite(enable1Pin, leftPWM);
    ledcWrite(enable2Pin, rightPWM);
}

void stopMotors() {
     //stops motion
    ledcWrite(enable1Pin, 0);
    ledcWrite(enable2Pin, 0);
    //disables motor
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, LOW);
}
////////////////
// MAIN LOOP //
///////////////

void loop() {
    char incomingPacket[255];  // buffer for incoming packets
    int packetSize = udp.parsePacket();

    if (packetSize) {
        int len = udp.read(incomingPacket, 255);
        if (len > 0) incomingPacket[len] = '\0';  // null terminate

        int leftPWM = 0;
        int rightPWM = 0;
    
        if (sscanf(incomingPacket, "%d %d", &leftPWM, &rightPWM) != 2) {
            Serial.println("Malformed packet");
            return; //skips this set of inputs
        }

        //HALT SECTION
        if (leftPWM == -1 && rightPWM == -1) {
            halted = true;
            stopMotors();
            Serial.println("HALTED");
        }
        //UNHALT SECTION
        else if (leftPWM == -2 && rightPWM == -2) {
            halted = false;
            Serial.println("UNHALTED");
        }
        //PWM SECTION
        else {
            if (!halted) {
                //Should be receiving in this range but mainting a safety check regardless
                if (leftPWM > 255 || rightPWM > 255 || leftPWM < 0 || rightPWM < 0) {
                    Serial.println("OUTSIDE EXPECTED VALUES, NEED TO CONSTRAIN");
                }
                leftPWM = constrain(leftPWM, 0, 255);
                rightPWM = constrain(rightPWM, 0, 255);
                Serial.printf
                ("Applying PWM: Left = %d, Right = %d\n",leftPWM,rightPWM);
                startMotors(leftPWM, rightPWM);
            } else {
                Serial.println("System Halted: skipping PWM command");
            }
        }
        
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