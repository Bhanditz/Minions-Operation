#define WT588D_RST 2 //7 //Module pin "REST" or pin # 1
#define WT588D_BUSY 3 //10 //Module pin "LED/BUSY" or pin # 15
#define WT588D_DATA 4 //8 //Module pin "P01" or pin # 9
#define WT588D_CS 5 //6 //Module pin "P02" or pin # 11
#define WT588D_CLK 6 //9 //Module pin "P03" or pin # 10

#define TWEEZER_PIN 7 //5 //Connect to the operation tweezers
#define BUZZER_PIN 8 //4 //Connect to the buzzer
#define NOSE_LED_PIN 9 //4 //Connect to the nose light LED

unsigned long startMillis = 0; //Variable to store the millis value
unsigned long currentMillis = 0; //Variable to store the millis on each loop
unsigned long idleTimeout = 120000; //Idle timeout duration in milliseconds
//unsigned long idleTimeout = 60000; //Idle timeout duration in milliseconds

void setup() {

  Serial.begin(9600);

  randomSeed(analogRead(0));

  pinMode(WT588D_RST, OUTPUT);  
  pinMode(WT588D_CS, OUTPUT); 
  pinMode(WT588D_CLK, OUTPUT); 
  pinMode(WT588D_DATA, OUTPUT); 
  pinMode(WT588D_BUSY, INPUT);  
  
  pinMode(TWEEZER_PIN, INPUT_PULLUP); //'Tweezer' Pin
  pinMode(BUZZER_PIN, OUTPUT); //'Buzzer' Pin
  pinMode(NOSE_LED_PIN, OUTPUT); //'Nose' Pin (Convert to an LED so we can drive it for as long as the sample is playing i.e. independantly of the motor)

  digitalWrite(WT588D_CS, HIGH);
  digitalWrite(WT588D_RST, HIGH);
  digitalWrite(WT588D_CLK, HIGH);

  digitalWrite(BUZZER_PIN, LOW); //Turn off buzzer
  digitalWrite(NOSE_LED_PIN, LOW); //Turn off buzzer

  //Reset WT588D
  digitalWrite(WT588D_RST, LOW);
  delay(50);
  digitalWrite(WT588D_RST, HIGH);

  startMillis = millis(); //Get the number of millis since the board was powered on
  Serial.print("millis at startup: ");
  Serial.println(startMillis);

}


void loop()
{

  /*
   * Need to monitor how long the system has been idle and start playing samples if it has been
   * left alone for too long. This is a nagging battery saver if the kids walk away from the game
   * then this will remind someone that it has been left on and hopefully someone will turn it off
   * 
   * How do we do that? 
   * 
   * Save the milliseconds since the board powered up at startup. 
   * At every loop (where nothing happens) check the difference between the saved and current millis
   * If the difference is > say 2 mins, play a sample (Random sample or a specific nag sample depending on space).
   * 
   * At every loop where something does happen, we save the current millis so the compare starts from that point
   */
  //When tweezer completes circuit to GND... (Active low as the internal pull up resistors will pull this line high when not connected)
  if(digitalRead(TWEEZER_PIN) == 0)
  { 
    Serial.println("Detected tweezer contact");
    
    //...buzz the buzzer and turn on 'nose' LED...
    
    Serial.println("Turn on buzzer and nose light...");
    
    digitalWrite(BUZZER_PIN, HIGH); //Turn on buzzer
    digitalWrite(NOSE_LED_PIN, HIGH); //Turn on LED
    
    //...and play a random sample.
    
    int randomSample = random(42);
    Serial.print("Playing random sample: ");
    Serial.println(randomSample);
    
    WT588D_Send_Command(randomSample);
    
    //Keep the buzzer on for a bit...
    delay(250);
    //...then turn it off.
    Serial.println("Turning off buzzer...");
    digitalWrite(BUZZER_PIN, LOW); //Turn off buzzer
    startMillis = millis(); //reset the idle timeout start because something has happened (tweezer contact)
    Serial.print("Resetting millis to current: ");
    Serial.println(startMillis);
  }

  //delay(50); //give the module time to start playing
  
  //This blocks while the sample is playing. 
  while(digitalRead(WT588D_BUSY) == 0)
  { 
    Serial.print("."); 
    delay(250);
  }
 
  //Once sample has finished playing, turn off the nose led.
  digitalWrite(NOSE_LED_PIN, LOW); //Turn off LED



  //!!!---TIMEOUT FOR NAG SAMPLE---!!!

  
  //Check how long it has been since nothing happend
  currentMillis = millis();
  if(currentMillis - startMillis >= idleTimeout) //Play nag tune if timeout exceeded
  {
    Serial.print("Timeout reached. millis: ");
    Serial.println(currentMillis);
    
    //Play the nag sample
    Serial.println("About to play nag tune...");
    Serial.println("Turning on nose LED");
    digitalWrite(NOSE_LED_PIN, HIGH); //Turn on LED
    int randomSample = random(42,45); //42, 43 & 44 (45 is never selected)
    
    Serial.print("Picking and playing RandomSample = ");
    Serial.println(randomSample);
    
    WT588D_Send_Command(randomSample); //nag songs are the last samples
    delay(250); //Delay to allow WT588D to start playing and activate the busy signal
    
    while(digitalRead(WT588D_BUSY) == 0)
    { 
      Serial.print("."); 
      delay(250);
      
      if(digitalRead(TWEEZER_PIN) == 0)
      { 
        Serial.println("Detected tweezer contact during nag tune...");
        //Reset WT588D. This causes the busy line to go low and so we break out of Nag sample if deliberate and sustained tweezer contact.
        digitalWrite(WT588D_RST, LOW);
        delay(50);
        digitalWrite(WT588D_RST, HIGH);
      }
    }
        
    Serial.println("WT588D reports no longer busy. Turning off nose LED...");
    digitalWrite(NOSE_LED_PIN, LOW); //Turn off LED
    Serial.println("Resetting timeout...");
    startMillis = millis(); //reset the idle timeout start
    Serial.println("All done here");
  }

}

//WT588D Code from http://forum.arduino.cc/?topic=227435.0

void WT588D_Send_Command(unsigned char addr) {

  unsigned char i;

  digitalWrite(WT588D_CS, LOW); 

  delay(5); //delay per device specifications

  for( i = 0; i < 8; i++)  {    
    digitalWrite(WT588D_CLK, LOW);    
    if(bitRead(addr, i))digitalWrite(WT588D_DATA, HIGH);
    else digitalWrite(WT588D_DATA, LOW);          
    delay(2);  //delay per device specifications   
    digitalWrite(WT588D_CLK, HIGH);    
    delay(2);  //delay per device specifications 
  } //end for

  digitalWrite(WT588D_CS, HIGH);

} //end WT588D_Send_Command
