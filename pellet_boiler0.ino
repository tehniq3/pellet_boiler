/*
 * automatic pellet boiler
 * concept: TommyPluss: http://tommypluss.blogspot.com/  
 * software: Nicu FLORICA (niq_ro): https://nicuflorica.blogspot.com/
 * ver.2d = simplified version: 17.1.2021, Craiova & Siminoc, Romania
 */


#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

#define ventilator 5  // PWM pin
#define motor 6       // 
#define bujie 7
#define remote 8
#define senzori 9   // DS18B20 sensors

#define adresaavarie 100


#define TEMPERATURE_PRECISION 9  
OneWire oneWire(senzori);  // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
DeviceAddress senzorcazan, senzorarzator, senzortub; // arrays to hold device addresses 

LiquidCrystal_I2C lcd(0x3F, 20, 4);  // // Set the LCD address to 0x27 for a 16 chars and 2 line display

unsigned long tpaerisire = 5000;  // time for clear the box
unsigned long tpalimentare0 = 5000; // time for add the fuel
unsigned long tpaprindere = 3000; // time for ignition
unsigned long tpslowspeed = 5000; // time for slow speed
unsigned long tpalimentare1 = 5000; // time for add the fuel
unsigned long tppauza1 = 60000; // time for wait to add the fuel
unsigned long tpalimentare2 = 5000; // time for add the fuel
unsigned long tppauza2 =120000; // time for wait to add the fuel
unsigned long tpoprire12;  // stop time for motor (pas 6 &7)

unsigned long tpcitire = 0;
unsigned long tppauza = 7000;  

byte slowspeed = 77;  // 77 is 30%, 51 is 20%
byte highspeed = 128;  // 128 is 50%, 179 is 70%

int teapaprag = 60;  // stop temperature for water
int deteapa = 3;  // hysteresis temperature for water
int teapa = 0;   // temperature for water
int tearzator = 0;   // temperature for burner
int tetub = 0;   // temperature for feeding system
int teapamax = 100.;  // maximum temperature for water
int tearzatormax = 100;  // maximum temperature for burner
int tetubmax = 100;  // maximum temperature for feeding system

byte comanda = 0;  // 0 - heater is off, 1 - heater in on
byte comanda0 = 3; 
byte slip = 1;  // 1 = stop, 0 - work
byte pas = 0;  // usual 6 or 7
byte pas2 = 1;  // usual 1 or 2
byte pasold = 0; // usual 6 or 7

byte test = 1;  // 1 = no sensors, 0 - normal mode
byte avarie = 0;  // 1 - problem
byte avarie0 = 1;
byte avarie1 = 1;
byte avarie2 = 1;
byte avarie3 = 1;

byte cazan[] = {
  B11110,
  B00001,
  B01110,
  B10000,
  B01110,
  B00001,
  B11110,
  B00000
};
/*
byte arzator[] = {
  B00010,
  B00100,
  B01100,
  B10010,
  B10001,
  B10001,
  B01110,
  B00000
};
*/
byte arzator[] = {
  B11111,
  B11111,
  B01110,
  B00000,
  B00110,
  B01100,
  B10010,
  B01100
};

byte alimentare[] = {
  B00010,
  B01000,
  B00010,
  B00100,
  B10001,
  B11111,
  B11111,
  B01110
};

byte bujia[] = {
  B00100,
  B00100,
  B01010,
  B01010,
  B01110,
  B10001,
  B10001,
  B01110
};

byte motor1[] = {
  B00000,
  B00111,
  B01100,
  B10010,
  B01100,
  B00111,
  B00000,
  B00000
};

byte motor2[] = {
  B00000,
  B11100,
  B00110,
  B01001,
  B00110,
  B11100,
  B00001,
  B00011
};


byte ventilatorul[] = {
  B00000,
  B00100,
  B00100,
  B01010,
  B10001,
  B00100,
  B00100,
  B11111
};




void setup()
{   
Serial.begin(9600);  // start serial port

//EEPROM.update(adresaavarie,0); 
  
lcd.begin();  // initialize the LCD
//  lcd.init();   // initialize the LCD

lcd.createChar(0, cazan);
lcd.createChar(1, arzator);
lcd.createChar(2, alimentare);
lcd.createChar(3, bujia);
lcd.createChar(4, ventilatorul);
lcd.createChar(5, motor1);
lcd.createChar(6, motor2);

lcd.backlight(); // Turn on the blacklight and print a message.
lcd.setCursor(0,0);
lcd.print("    Automatizare    ");
lcd.setCursor(0,1);
lcd.print("cazan peleti v.1.1.0");
lcd.setCursor(0,2);
lcd.print("concept: TommyPluss ");
lcd.setCursor(0,3);
lcd.print("program:Nicu FLORICA");

pinMode(ventilator, OUTPUT);
pinMode(bujie, OUTPUT);
pinMode(motor, OUTPUT);

pinMode(remote, INPUT);
digitalWrite(remote, HIGH);  // pullup the input for remote

if (test != 1)
{
sensors.begin();   // Start up the library for DS18B20 sensors
 // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");  
  if (!sensors.getAddress(senzorcazan, 0)) Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(senzorarzator, 1)) Serial.println("Unable to find address for Device 1");
  if (!sensors.getAddress(senzortub, 2)) Serial.println("Unable to find address for Device 2");
  Serial.print("Device 0 Address: ");  // show the addresses we found on the bus
  printAddress(senzorcazan);
  Serial.println();
  Serial.print("Device 1 Address: ");
  printAddress(senzorarzator);
  Serial.println();
  Serial.print("Device 2 Address: ");
  printAddress(senzortub);
  Serial.println();
  sensors.setResolution(senzorcazan, TEMPERATURE_PRECISION);  // set the resolution to 9 bit per device
  sensors.setResolution(senzorarzator, TEMPERATURE_PRECISION);
  sensors.setResolution(senzortub, TEMPERATURE_PRECISION);
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(senzorcazan), DEC);
  Serial.println();
  Serial.print("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(senzorarzator), DEC);
  Serial.println();
  Serial.print("Device 2 Resolution: ");
  Serial.print(sensors.getResolution(senzortub), DEC);
  Serial.println();
}
//delay(2000);
//lcd.clear();
  Serial.println(" ");
  Serial.println("pas.1 - pornire sistem");
//  lcd.setCursor(0,0);
//  lcd.print("pas.1");

delay(4000);
lcd.clear();

lcd.setCursor(0,0);
lcd.print("1: pornire sistem");
delay(2000);
lcd.clear();

avarie = EEPROM.read(adresaavarie);

while (avarie == 1)  // if was a danger situation
{
  temperaturi();
  if (teapa < teapaprag) avarie1 = 0;
     else avarie1 = 1;
  if (tearzator < teapaprag) avarie2 = 0;
     else avarie2 = 1;
  if (tetub < teapaprag) avarie3 = 0;
     else avarie3 = 1;
  avarie0 = avarie1 + avarie2 + avarie3;
  if (avarie0 == 0)
  {
    avarie = 0;
    EEPROM.update(adresaavarie,0); 
    lcd.clear();
  }
  else
  {
   lcd.setCursor(0,0);
   lcd.print("Temperatura depasita"); 
   lcd.setCursor(0,1);
   lcd.print("--------------------");
   delay(5000);  // check every 5 seconds
  }
}


} // end setup

void loop()
{

if (avarie == 0)
{
comanda = digitalRead(remote);  // read the status of remote control
/*
Serial.print("Comanda = ");
Serial.println(comanda);
*/

if (millis() - tpcitire > tppauza)
{
temperaturi();
temperaturi2();
}

comanda = digitalRead(remote);  // read the status of remote control
if (comanda != comanda0)
{
Serial.print("Comanda = ");
Serial.println(comanda);
}

if (comanda == HIGH)  // remotecontrol 
{
if (comanda != comanda0)
{
 Serial.print("Comanda termostat ! "); 
}
while (slip == 1) // initial command is off
{
 aprindere();
  slip = 0;
}


// step 6
if (pas == 6)
{
/*
Serial.println("---------------------");
Serial.println("pas.6 !");
*/
lcd.setCursor(0,0);
lcd.print("6: regim normal  ");
if (pas2 == 1)
{
Serial.println("pas.6 - alimentare cu peleti");
analogWrite(ventilator, highspeed);  // fan at high speed
    lcd.setCursor(18, 0);
    lcd.write(4);
    lcd.write(4);
digitalWrite(motor, HIGH);  // power the motor
    lcd.setCursor(0,1);
    lcd.print("                    ");
    lcd.setCursor(16, 0);
    lcd.write(5);
    lcd.write(6);
delay(tpalimentare1); // time for add the fuel
digitalWrite(motor, LOW);  // stop the motor
    lcd.setCursor(16, 0);
    lcd.print("  ");
tpoprire12 = millis();
pas2 = 2;
Serial.println("pas.6 - pauza");
}
 if (millis() - tpcitire > tppauza)  // read the temperatures
  {
  temperaturi();
  temperaturi2();
  }
if ((pas2 == 2) and (millis() - tpoprire12 > tppauza1))
{
 pas2 = 1;
}
} // end pas = 6

// step 7
if (pas == 7)
{
/*
Serial.println("---------------------");
Serial.println("pas.7 !");
*/
lcd.setCursor(0,0);
lcd.print("7: regim veghe   ");
if (pas2 == 1)
{
//Serial.println("pas.7 - alimentare");
analogWrite(ventilator, slowspeed);  // fan at slow speed
    lcd.setCursor(18, 0);
    lcd.print(" ");
    lcd.setCursor(19, 0);
    lcd.write(4);
digitalWrite(motor, HIGH);  // power the motor
    lcd.setCursor(0,1);
    lcd.print("                    ");
    lcd.setCursor(16, 0);
    lcd.write(5);
    lcd.write(6);
delay(tpalimentare2); // time for add the fuel
Serial.println("pas.7 - alimentare cu peleti");
digitalWrite(motor, LOW);  // stop the motor
    lcd.setCursor(16, 0);
    lcd.print("  ");
tpoprire12 = millis();
pas2 = 2;
Serial.println("pas.7 - pauza");
}
    lcd.setCursor(18, 0);
    lcd.print(" ");
    lcd.setCursor(19, 0);
    lcd.write(4);
 if (millis() - tpcitire > tppauza)  // read the temperatures
  {
  temperaturi();
  temperaturi2();
  }
if ((pas2 == 2) and (millis() - tpoprire12 > tppauza2))
{
 pas2 = 1;
}

} // end pas = 7
/*
Serial.print("slip = ");
Serial.println(slip);
*/
lcd.setCursor(0,1);
    lcd.write(5);
    lcd.write(6); 
    lcd.print(":");
if((millis()-tpoprire12)/1000 < 100) lcd.print(" ");
if((millis()-tpoprire12)/1000 < 10) lcd.print(" ");
lcd.print((millis()-tpoprire12)/1000);
lcd.print("s pauza");

pasold = pas;
} // end comanda = 1;


if (comanda == 0)
{
if (comanda != comanda0)
{
lcd.clear();
Serial.println("---------------------");
Serial.println("off ?");
}
lcd.setCursor(0,0);
lcd.print("0: regim asteptare  ");
digitalWrite(bujie, LOW);  // stop the ignition
analogWrite(ventilator, 0);  // fan is stop
digitalWrite(motor, LOW);  // stop the motor
slip = 1;  // system go to sleep
} // end  comanda = 0;

delay(100);
comanda0 = comanda;
}  // end avarie = 0;

if (avarie == 1)
{
digitalWrite(bujie, LOW);  // stop the ignition
analogWrite(ventilator, 0);  // fan is stop
digitalWrite(motor, LOW);  // stop the motor
 lcd.setCursor(0,0);
 lcd.print("----- Avarie! ------");
 lcd.setCursor(0,1);
 lcd.print("--------------------");
if (millis() - tpcitire > tppauza)
{
temperaturi();
}
 delay(100); 
}  // end avarie = 1

}  // end main loop


void aprindere() // steps 2..5
{
  // step 2
Serial.println("---------------------");
Serial.println("pas.2  - ventilator la turatie maxima");
lcd.setCursor(0,0);
lcd.print("2: aerisire      ");
analogWrite(ventilator, 255);  // fan at maximum
    lcd.setCursor(17, 0);
    lcd.write(4);
    lcd.write(4);
    lcd.write(4);   
delay(tpaerisire);  // time for clear the box
    lcd.setCursor(17, 0);
    lcd.print("   ");

// step 3
Serial.println("---------------------");
Serial.println("pas.3 - opresc ventilatorul, pornesc snecul");
lcd.setCursor(0,0);
lcd.print("3: alimentare    ");
analogWrite(ventilator, 0);  // fan is off
digitalWrite(motor, HIGH);  // power the motor
    lcd.setCursor(17, 0);
    lcd.write(5);
    lcd.write(6);
delay(tpalimentare0); // time for add the fuel
    lcd.setCursor(17, 0);
    lcd.print("   ");

// step 4
Serial.println("---------------------");
Serial.println("pas.4 - opresc motorul, aprind bujia");
lcd.setCursor(0,0);
lcd.print("4: aprindere bujie");
digitalWrite(motor, LOW);  // stop the motor
digitalWrite(bujie, HIGH);  // power the ignition
    lcd.setCursor(19, 0);
    lcd.write(3);
delay(tpaprindere); // time for ignnition
    lcd.setCursor(19, 0);
    lcd.print(" ");

// step 5 
Serial.println("---------------------");
Serial.println("pas.5 - sting bujia, ventilator la turatie mica");
lcd.setCursor(0,0);
lcd.print("5: mentinere foc   ");
digitalWrite(bujie, LOW);  // stop the ignition
analogWrite(ventilator, slowspeed);  // fan at slow speed
    lcd.setCursor(19, 0);
    lcd.write(4);
delay(tpslowspeed); // time for slow speed
analogWrite(ventilator, highspeed);  // fan at high speed
    lcd.setCursor(18, 0);
    lcd.write(4);
    lcd.write(4);    
Serial.println("ventilator la turatie mare");
}  // end aprindere


// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void temperaturi()
{
if (test != 1)
{
Serial.print("Requesting temperatures...");
sensors.requestTemperatures();
Serial.println("DONE");

teapa = sensors.getTempC(senzorcazan);
lcd.setCursor(0,1);
if (teapa < 10) lcd.print(" ");
lcd.print(teapa,1);
lcd.print(char(223));
lcd.print("C ");
  if(teapa == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
Serial.print("Sensor 0 = ");
Serial.print(teapa);
Serial.println("`C");
tearzator = sensors.getTempC(senzorarzator);
  if(teapa == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
Serial.print("Sensor 1 = ");
Serial.print(tearzator);
Serial.println("`C");
tetub = sensors.getTempC(senzortub);
  if(tetub == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
Serial.print("Sensor 2 = ");
Serial.print(tetub);
Serial.println("`C");
}
else
{
teapa = 20 + random(85);
Serial.print("Temperatura apa = ");
Serial.print(teapa);
Serial.println("`C");
tearzator = 20 + random(85);
tetub = 20 + random(85);
}
    lcd.setCursor(0, 2);
    lcd.print("  apa  arzator  snec");
    if (teapa < 0) teapa = 0;
    lcd.setCursor(0, 3);
    lcd.write(0);
    if (teapa< 100) lcd.print(":");
    if (teapa < 10) lcd.print(" ");
    lcd.print(teapa);
    lcd.print(char(223));
    lcd.print("C ");   

    if (tearzator < 0) tearzator = 0;
    lcd.setCursor(7, 3);
    lcd.write(1);
    if (tearzator< 100) lcd.print(":");
    if (tearzator < 10) lcd.print(" ");
    lcd.print(tearzator);
    lcd.print(char(223));
    lcd.print("C ");   

    if (tetub < 0) tetub = 0;
    lcd.setCursor(14, 3);
    lcd.write(2);
    if (tetub < 100) lcd.print(":");
    if (tetub < 10) lcd.print(" ");
    lcd.print(tetub);
    lcd.print(char(223));
    lcd.print("C ");

tpcitire = millis();
//delay(1000);
} // end temperaturi

void temperaturi2()
{
if (teapa < teapaprag - deteapa)  // pas 6
{
Serial.println("---------------------");
Serial.println("urmeaza pas.6");
/*
lcd.setCursor(0,0);
lcd.print("6");
*/
pas = 6;

if (pasold != pas) // change the mode
{
  if (millis() - tpoprire12 > tppauza1)   pas2 = 1;
  pasold = pas;
}
}

if (teapa >= teapaprag) // pas 7
{
Serial.println("---------------------");
Serial.println("urmeaza pas.7");
/*
lcd.setCursor(0,0);
lcd.print("7");
*/
pas = 7;

if (pasold != pas) // change the mode
{
  if (millis() - tpoprire12 > tppauza2)   pas2 = 1;
  pasold = pas;
}
}

/*
if (pasold != pas) // change the mode
{
 // tpoprire1 = 0;
 // tpoprire2 = 0;
  pas2 = 1;
}
*/

if (teapa >= teapamax) // avarie
{
Serial.println("---------------------");
Serial.println("avarie, temperatura apa in cazan");
lcd.clear();
avarie = 1; 
tpcitire = 0;
EEPROM.update(adresaavarie,1); 
lcd.clear();
}
if (tearzator >= tearzatormax) // avarie
{
Serial.println("---------------------");
Serial.println("avarie, temperatura mare la arzator");
lcd.clear();
avarie = 1; 
tpcitire = 0;
EEPROM.update(adresaavarie,1); 
}
if (tetub >= tetubmax) // avarie
{
Serial.println("---------------------");
Serial.println("avarie, temperatura mare la tub alimentare");
lcd.clear();
tpcitire = 0;
avarie = 1;
EEPROM.update(adresaavarie,1); 
}
Serial.println("Masurare temperaturi - OK !");
}  // end temparaturi2
