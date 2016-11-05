/* LaserTag
 *  ws2812 on pin 11
 *  Laser on pin 3
 *  IR sensor on pin 2
 *  trigger switch on pin 4 (5 ground)
 */
//Decoded Sony(2): Value:52E9 Adrs:0 (15 bits) RED
//Decoded Sony(2): Value:32E9 Adrs:0 (15 bits) GREEN
//Decoded Sony(2): Value:72E9 Adrs:0 (15 bits) YELLOW
//Decoded Sony(2): Value:12E9 Adrs:0 (15 bits) BLUE


#include <NeoPixelBus.h>
const uint16_t PixelCount = 1; // this example assumes 4 pixels, making it smaller will cause a failure
const uint8_t PixelPin = 11;  // make sure to set this to the correct pin, ignored for Esp8266

NeoPixelBus<NeoGrbFeature,Neo800KbpsMethod> strip(PixelCount, PixelPin);
 
#include <IRLibDecodeBase.h> // First include the decode base
#include <IRLibSendBase.h>    // First include the send base

#include <IRLib_P02_Sony.h>  // to actually use. The lowest numbered
#include <IRLib_P11_RCMM.h>
#include <IRLibCombo.h>     // After all protocols, include this
// All of the above automatically creates a universal decoder
// class called "IRdecode" containing only the protocols you want.
// Now declare an instance of that decoder.
IRdecode myDecoder;
IRsend mySender;

// Include a receiver either this or IRLibRecvPCI or IRLibRecvLoop
#include <IRLibRecv.h> 
IRrecv myReceiver(2);  //pin number for the receiver


int hit_count = 0;

float brite = 0.05f;

HslColor red(0.0, 1, brite);
HslColor yellow(0.16, 1, brite);
HslColor green(0.33, 1, brite );
HslColor blue(0.66,  1, brite);
HslColor white(0, 0, brite);
HslColor black(0);
HslColor currentColor = HslColor();
float temp_hue;

void set_colour(HslColor blah) {
  currentColor = blah;
  strip.SetPixelColor(0, currentColor);
  strip.Show();  
}

void setup() {

  pinMode(5,OUTPUT);    // switch ground
  digitalWrite(5,LOW);
  pinMode(4,INPUT);     // switch input
  digitalWrite(4,HIGH); // with pullup

  strip.Begin();

  set_colour(red);

  Serial.begin(115200);
  delay(2000); while (!Serial); //delay for Leonardo

  set_colour(green);

  myReceiver.enableIRIn(); // Start the receiver
  Serial.println(F("Ready to receive IR signals"));
}

char last_button;
char this_button;

char ser;
int32_t code;
int len;
unsigned long last_millis;

void do_button() {
  if ( (millis() - last_millis) > 100 ) {
    last_millis = millis();
    this_button = digitalRead(4);
    if ((this_button == 0) && (last_button == 1)) {
      code = 0x290;  len=12; // mute
      mySender.send(SONY, code, len);
      myReceiver.enableIRIn();      //Restart receiver      
    }
    last_button = this_button;
  }
}


void do_transmit() {
  if (Serial.available()) {
    ser = Serial.read();
    code = -1;
    
    switch (ser) {
      case 'r':
        code = 0x52E9;  len=15;
        break;
      case 'g':
        code = 0x32E9;  len=15;
        break;
      case 'b':
        code = 0x72E9;  len=15;
        break;
      case 'y':
        code = 0x12E9;  len=15;
        break;
      case 'm':
        code = 0x290;  len=12; // mute
        break;
      default:
        break;
    }
    if (code >= 0) {
      mySender.send(SONY, code, len);
      myReceiver.enableIRIn();      //Restart receiver
      Serial.print("key=");      Serial.print(ser);
      Serial.print(" code=");    Serial.println(code,HEX);
    }
  }  
}

void do_receive() {
  //Continue looping until you get a complete signal received
  if (myReceiver.getResults()) { 
    myDecoder.decode();           //Decode it
    if (myDecoder.protocolNum == SONY) {
      switch (myDecoder.value) {
        case 0x52E9:
          Serial.println("got red.");
          set_colour(red);
        break;
        case 0x32E9:
          Serial.println("got green.");
          set_colour(green);
        break;
        case 0x72E9:
          Serial.println("got yellow.");
          set_colour(yellow);
        break;
        case 0x12E9:
          Serial.println("got blue.");
          set_colour(blue);
        break;
        case 0x290:
          Serial.println("got mute.");
          hit_count+=1;
          temp_hue = float(hit_count%60)/60;
          set_colour(HslColor(temp_hue, 1, brite));
          Serial.print("hit_count: ");  Serial.println(hit_count);
          Serial.print("hue: ");  Serial.println(temp_hue);
        break;
        default:
          myDecoder.dumpResults(false);  //Now print results. Use false for less detail
      }
    }
    myReceiver.enableIRIn();      //Restart receiver
  }
}
void loop() {
  do_button();
  do_receive();
  do_transmit();
}

