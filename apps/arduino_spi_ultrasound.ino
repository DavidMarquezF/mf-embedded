
// Written by Nick Gammon
// April 2011


#define echoPin 2 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 3 //attach pin D3 Arduino to pin Trig of HC-SR04
// what to do with incoming data
volatile byte command = 0;

// defines variables
long duration; // variable for the duration of sound wave travel
double distance; // variable for the distance measurement

void setup (void)
{
  Serial.begin(9600);

  // HC-SR04
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT


  //------------------SPI------------

  pinMode(MOSI, OUTPUT);
  pinMode(SCK, INPUT);
  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);

  // turn on SPI in slave mode
  SPCR |= _BV(SPE);

  // turn on interrupts
  SPCR |= _BV(SPIE);

  //-------------------

}  // end of setup




volatile int pos;
volatile char cmd;

#define INVALID_CMD 'z'

// SPI interrupt routine
ISR (SPI_STC_vect)
{
  byte c = SPDR;
  if(c == 'g' || c == 'i'){
    cmd = c;
    pos = 0;
    SPDR=0;
    return;
  }
  
  if(cmd == INVALID_CMD){
    SPDR = 0;
    return;
  }
  if(cmd == 'g'){
    const byte * buf = (const byte*)&distance;
    SPDR = buf[pos];
    if(++pos >= sizeof(double)) // Don't do sizeof(buf) since it is a pointer
      cmd = INVALID_CMD;  
  }
  else if(cmd == 'i'){
    SPDR = 1; // Ultrasound id is 1
    cmd = INVALID_CMD;
  }
  
}  // end of interrupt service routine (ISR) SPI_STC_vect

void loop (void)
{
  
 // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  //Serial.print("Distance: ");
  //Serial.print(distance);
  //Serial.println(" cm");
}  // end of loop