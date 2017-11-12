
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

uint8_t id;

uint8_t getFingerprintEnroll();

// pin 2 is IN from sensor (green wire)
// pin 3 is OUT from arduino  (white wire)
SoftwareSerial mySerial(2, 3);

//Init finger class
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


void setup()  
{
  while (!Serial);  // Some controllers require this instruction, can be useful...
  delay(500);
  
  Serial.begin(9600); //Begin arduino console to display some results and make sure everything works
  Serial.println("Adafruit Fingerprint sensor enrollment");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  
  //Verify if there is the sensor or not
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);
  }
}


//Function to get a number via console to save the ID (Must be improved to generate the id itself)
uint8_t readnumber(void) {
  uint8_t num = 0;
  boolean validnum = false; 
  while (1) {
    while (! Serial.available());
    char c = Serial.read();
    if (isdigit(c)) {
       num *= 10;
       num += c - '0';
       validnum = true;
    } else if (validnum) {
      return num;
    }
  }
}

void loop()                     // run over and over again
{
  uint8_t mode;
  Serial.println("Do you want to enroll a new fingerprint (1) or to validate users (2)? ");
  mode = readnumber();
  if (mode == 1) {
    Serial.println("Ready to enroll a fingerprint! Please Type in the ID # you want to save this finger as...");
    id = readnumber();
    Serial.print("Enrolling ID #");
    Serial.println(id);
    
    while (!  getFingerprintEnroll() );
  } else {
    while(true) {
      getFingerprintIDez();
      delay(50);
    }
  }
}

uint8_t uploadFingerpintTemplate(uint16_t id, uint8_t templateBuffer[])
{
 uint8_t p = finger.loadModel(id);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("template "); Serial.print(id); Serial.println(" loaded");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    default:
      Serial.print("Unknown error "); Serial.println(p);
      return p;
  }
        
  // OK success!

  p = finger.getModel();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("template "); Serial.print(id); Serial.println(" transferring");
      break;
   default:
      Serial.print("Unknown error "); Serial.println(p);
      return p;
  }
  
  int index=0;
  uint32_t starttime = millis();
  while ((index < 256) && ((millis() - starttime) < 1000))
  {
    if (mySerial.available())
    {
      templateBuffer[index] = mySerial.read();
      index++;
    }
  }
  
  Serial.print(index); Serial.println(" bytes read");
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    tone(9,261, 500);
    delay(100);
    noTone(9);
    tone(9,261, 500);
    delay(100);
    noTone(9);
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    tone(9, 277, 500);
    delay(200);
    noTone(9);
    tone(9, 349, 500);
    delay(200);
    noTone(9);  
    tone(9, 440, 500);
    delay(200);
    noTone(9);
  } else {
    Serial.println("Internar error");
  }   
  
  uint8_t templateBuffer[256];
  memset(templateBuffer, 0xff, 256);  //zero out template buffer
  
  uploadFingerpintTemplate(id, templateBuffer);
  //dump entire templateBuffer.  This prints out 16 lines of 16 bytes
  
  int count;
  for (count= 0; count < 16; count++)
  {
    for (int i = 0; i < 16; i++)
    {
      Serial.print("0x");
      Serial.print(templateBuffer[count*16+i], HEX);
      Serial.print(", ");
    }
    Serial.println();
  }
  
  //SAVE TEMPLATE TO EEPROM
  //THAT SHOULD WORK...
  int number_templates = EEPROM.read(0);
  Serial.print("HI HA AQUESTS TEMPLATES: ");
  Serial.print(number_templates);
  Serial.println();
  if (number_templates == 255) {
    number_templates = 0;
  }
  EEPROM.write(number_templates*257 + 1, id);
  for (count= 0; count < 256; count++)
  {
    EEPROM.write(number_templates*257 + 2 + count, templateBuffer[count]);
  }
  EEPROM.write(0, number_templates + 1);
  
  Serial.print("S'ha guardat la ID: ");
  Serial.print(EEPROM.read(1), DEC);
  for (count= 2; count < 258; count++)
  {  
      Serial.print("0x");
      Serial.print(EEPROM.read(count), HEX);
      Serial.print(", ");
  }
  
  /*EEPROM.write(1, id);
  Serial.print("S'ha guardat la ID: ");
  Serial.print(EEPROM.read(1), DEC);*/
  
  //Save Buffer With The ID HERE in EEPROM
  
  //we need to divide templateBuffer, because if we want to save it, the sensor admits packets of max 128 bytes (2 datapackets needed)
  
  
  
  
  
  
  
  
  
  
  // CODE TO PUT A TEMPLATE WITH AN ID TO SENSOR
  
  /*uint8_t dataPacket1[128];
  memset(dataPacket1, 0xff,128);
  uint8_t dataPacket2[128];
  memset(dataPacket2, 0xff, 128);
  
  for (count= 0; count < 128; count++)
  {
    dataPacket1[count] = templateBuffer[count];
  }
  
  for (count= 0; count < 128; count++)
  {
    dataPacket2[count] = templateBuffer[count + 128];
  }
  
  //BORRAR LA BASE DE DADES SENCERA
  finger.emptyDatabase();
  
  Serial.println("Deleting Database");
  
  p = finger.downloadModel(id, dataPacket1, dataPacket2, 1);
  if (p == FINGERPRINT_OK) {
    Serial.println("Template saved to Buffer1");
  } else {
    Serial.println("Error saving template to Buffer1");
  }
  p = finger.downloadModel(id, dataPacket1, dataPacket2, 2);
  if (p == FINGERPRINT_OK) {
    Serial.println("Template saved to Buffer2");
  } else {
    Serial.println("Error saving template to Buffer2");
  }
  
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
  
  memset(templateBuffer, 0xff, 256);  //zero out template buffer
  uploadFingerpintTemplate(id, templateBuffer);
  //dump entire templateBuffer.  This prints out 16 lines of 16 bytes
  
  for (count= 0; count < 16; count++)
  {
    for (int i = 0; i < 16; i++)
    {
      Serial.print("0x");
      Serial.print(templateBuffer[count*16+i], HEX);
      Serial.print(", ");
    }
    Serial.println();
  }*/
}





//Functions to get the id

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence); 
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}
