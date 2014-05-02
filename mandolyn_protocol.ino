const unsigned int SIGNALPIN = 7;
const unsigned int LEDPIN = 6; 
const unsigned int BITS = 36;
const unsigned int ONEMS = 900;//uS Tweeking
const String c1 = "1100";
const String c2 = "110";
unsigned int cycle;
unsigned int cycleStartTime;
unsigned int preCycleStartTime;
unsigned int deltaTime;

double previosTemp = 0;
String priviosTempBitpattern = "";

int previosSensorID = 0;
int previosSensorRH = 0;
double previosSensorTemp = 0;
String priviosSensorPattern = "";

double previosRH = 0;
String priviosRHBitpattern = "";

int startUs;
int endUs;

boolean state;
boolean cyclesLeft;

char buffer[BITS+1];


void setup(){
pinMode(SIGNALPIN, OUTPUT);
pinMode(LEDPIN, OUTPUT);
digitalWrite(SIGNALPIN, LOW);
digitalWrite(LEDPIN, LOW);

Serial.begin(9600);
}
void loop(){
Serial.println("Start");
startUs = millis();
sendSensorSignalToTellstick(123,88,12.1);
//getTempPattern(12.1);
endUs = millis();
Serial.println("END in us:"+ String(endUs-startUs));
delay(1000);
}

const void sendSensorSignalToTellstick(const unsigned int ID,const unsigned int RH,const double Temp){
  String pat;
  if(previosSensorID == ID && previosSensorRH == RH & previosSensorTemp == Temp){pat = priviosSensorPattern;}
  else{
  pat = c1 + telldusID(ID) + c2 + getRHPattern(RH) + getTempPattern(Temp);
  previosSensorID = ID;
  previosSensorRH = RH;
  previosSensorTemp = Temp;
  priviosSensorPattern = pat;
  }
  sendRAW(pat);
} 

const String telldusID(const unsigned int ID){
  unsigned int houseCode = ID/10;
  unsigned int chanalCode = ID - (houseCode*10);
  if(chanalCode > 4 && chanalCode < 1){
  chanalCode = 1;
  // Unvalid chanal code
  }
  if(houseCode > 16 && houseCode < 0){
  houseCode = 0;
  // Unvalid Housecode code
  }
  
  //Serial.println(String(houseCode) + String(chanalCode));
  return getHouseCodePattern(houseCode) + getChanaleCodePattern(chanalCode);
}

String getHouseCodePattern(const unsigned int housecode){
  String hc_pat;
  hc_pat = intToBinString( housecode, 4);
  return hc_pat; 
}

String getChanaleCodePattern(const unsigned int chanalcode){
  String ch_pat;
  ch_pat = intToBinString( chanalcode-1, 2);
  return ch_pat; 
}

String getTempPattern(double temp){
  if(previosTemp == temp){ return priviosTempBitpattern;}
  else{
  String result;
  int tempNumber = (temp * 128) + 6400;
  result = intToBinString(tempNumber, 15);
  
  previosTemp = temp;
  priviosTempBitpattern = result;
  return result;
  }
}

String getRHPattern(const unsigned int RH){
  if(previosRH == RH){return priviosRHBitpattern;}
  else{
  String pat = "";
  pat = intToBinString(RH, 7);
  previosRH = RH;
  priviosRHBitpattern = pat;
  return pat;
  }
}

String intToBinString(const unsigned int number, const unsigned int numberOfBits){
  String binString = "";
  unsigned int _tempNumber = number;
  //Serial.println("test " + String(number));
  
  for(int i = numberOfBits-1; i >= 0; i--){
    if((_tempNumber - powOfTow(i)) >= 0 && _tempNumber != 0){
     binString += "1";
     _tempNumber -= powOfTow(i);
    }else{
    binString += "0";
    }
    
  }
  
  return binString;
}

void sendRAW(const String pat){
  
  // init variables
  cycle = 0;
  cyclesLeft = true;
  state = 0;
  
  // setting start time
  preCycleStartTime = micros();
  
  // Bit string to char array
  pat.toCharArray(buffer, BITS);
  
  // Addinf getting check bit in the end
  buffer[BITS-1] = getCheckBit(buffer, BITS);
  
  while(cyclesLeft){ // starting to send pattern
    cycleStartTime = micros();
    if((cycleStartTime - preCycleStartTime) >= ONEMS){
      preCycleStartTime = cycleStartTime;
      if(buffer[cycle/2] == '1'){
          state = !state; // switches state in the middle of the cycle
      }else{
        if((cycle+1)%2){
          state = !state; // switches state on the start of the cylce
        }
      }
      digitalWrite(SIGNALPIN, state);
      cycle++;
    }
    if(cycle >= ((BITS) * 2)+3){ // adds a '0' on the end of the pattern
      delayMicroseconds(ONEMS*2);
      cyclesLeft = false; // stops while loop
      digitalWrite(SIGNALPIN, LOW); // ends signal by setting pin to low
    }
  }
  delay(1);
  digitalWrite(SIGNALPIN, LOW);
  digitalWrite(LEDPIN, LOW);
}

char getCheckBit(char pat[], int length){
  unsigned int result = 0;
  for(int i = 0; i < length; i++){ // counting number of ones in pattern
    if(pat[i] == '1'){result++;}
  }
  return intToChar(result%2);  // if odd return '1' else return '0'
}

const char intToChar(const int number){
  switch (number){
    case 1:
      return '1';
      break;
    case 0:
      return '0';
      break;
    default:
      return 'E';
      break;  
  }
}

const int powOfTow(const unsigned int pow){
  unsigned int result = 0;
  for(int i = 0; i < pow; i++){
    if(result == 0){
      result = 2;
    }else{
      result = result * 2;
    }
  }
  return result;
}
