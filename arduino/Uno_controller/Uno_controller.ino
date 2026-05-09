#include <SoftwareSerial.h>
#include <StandardCplusplus.h>
#include <vector>

#define commandTime 100

class Command{
  private:
  signed int m_x;
  signed int m_y;
  int m_timeRemained;

  public:
  Command(signed int x, signed int y, int timeRemained){
    m_x = x;
    m_y = y;
    m_timeRemained = timeRemained;
  }

  signed int getX(){
    return m_x;
  }

  signed int getY(){
    return m_y;
  }

  int getTimeRemained(){
    return m_timeRemained;
  }

  void setX(signed int x){
    m_x = x;
  }

  void setY(signed int y){
    m_y = y;
  }

  void setTimeRemained(int timeRemained){
    m_timeRemained = timeRemained;
  }
  
};

class QueueManager{
  private:
  std::vector<Command> m_commandList;
  long lastEpoch;
  long elapsed;
  signed int m_currentX, m_currentY;

  public:
  QueueManager(){
    lastEpoch = 0;
    elapsed = 0;
    m_currentX = 0;
    m_currentY = 0;
  }
  
  void addCommand(signed int x, signed int y){
    m_commandList.push_back(Command(x, y, commandTime));
  }

  void setStep(){
    unsigned long currentTime = millis();
    if(lastEpoch != 0){
      elapsed = currentTime - lastEpoch;
      
    }
    lastEpoch = currentTime;
   
    //updating remaining times
    if(m_commandList.size() != 0){
      int tRemaining = m_commandList.at(0).getTimeRemained();
      if(tRemaining > elapsed){
        m_commandList.at(0).setTimeRemained(tRemaining - elapsed);
        m_currentX = m_commandList.at(0).getX();
        m_currentY = m_commandList.at(0).getY();
      }
      else{
        m_commandList.erase(m_commandList.begin());
        if(m_commandList.size() > 0){
          m_currentX = m_commandList.at(0).getX();
          m_currentY = m_commandList.at(0).getY();
        }
        else{
          m_currentX = 0;
          m_currentY = 0;
        }
      }
    }
  }

  signed int getCurrentX(){
    return m_currentX;
  }

  signed int getCurrentY(){
    return m_currentY;
  }
  
};

std::vector<String> split(String stringToBeSplitted, String delimeter)
{
     std::vector<String> splittedString;
     int startIndex = 0;
     int  endIndex = 0;
     while( (endIndex = stringToBeSplitted.indexOf(delimeter, startIndex)) < stringToBeSplitted.length() )
    {
       String val = stringToBeSplitted.substring(startIndex, endIndex);
       splittedString.push_back(val);
       startIndex = endIndex + delimeter.length();
     }
     if(startIndex < stringToBeSplitted.length())
     {
       String val = stringToBeSplitted.substring(startIndex);
       splittedString.push_back(val);
     }
     return splittedString;
}



#define RX 10
#define TX 11
#define enA 9
#define enB 5
#define inA1 6
#define inA2 7
#define inB1 3
#define inB2 4

#define maxSpeed 5

//Initializations
QueueManager queueManager;
SoftwareSerial mySerial(RX, TX); // RX, TX

void manageSerial(){
  String recieved = "";
   if (mySerial.available()) {
    delay(1);
     while(mySerial.available()) {
      recieved += (char)mySerial.read();
    }
  }

  if(recieved != ""){
    Serial.println(recieved);
  }

  // Multiple command
//  std::vector<String> commands = split(recieved, ",");
//  for(int i=0; i<commands.size(); i++){
//    int x = (commands.at(i).substring(commands.at(i).indexOf('X')+1,commands.at(i).indexOf('Y'))).toInt();
//    int y = (commands.at(i).substring(commands.at(i).indexOf('Y')+1)).toInt();
//    if(x != 0 || y != 0){
//      queueManager.addCommand(x, y);
//    }
//  }

  // Single command
   int x = (recieved.substring(recieved.indexOf('X')+1,recieved.indexOf('Y'))).toInt();
   int y = (recieved.substring(recieved.indexOf('Y')+1)).toInt();
   if(x != 0 || y != 0){
     queueManager.addCommand(x, y);
//   manageActuator(x, y);
   }
   
}

void actuatorForward(int speed){
  digitalWrite(inA1, LOW);
  digitalWrite(inA2, HIGH);
  int pwmOutput = map(speed, 0, maxSpeed, 0 , 255); // Map the potentiometer value from 0 to 5
  analogWrite(enA, pwmOutput); // Send PWM signal to L298N Enable pin
}

void actuatorBackward(int speed){
  digitalWrite(inA1, HIGH);
  digitalWrite(inA2, LOW);
  int pwmOutput = map(speed, 0, maxSpeed, 0 , 255); // Map the potentiometer value from 0 to 5
  analogWrite(enA, pwmOutput); // Send PWM signal to L298N Enable pin
}

void actuatorBreakX(){
  digitalWrite(inA1, LOW);
  digitalWrite(inA2, LOW);
  analogWrite(enA, 0); // Send PWM signal to L298N Enable pin
}

void actuatorLeft(int speed){
  // Because actuator can not work with less than speed of 5, speed is removed temporarily
  digitalWrite(inB1, LOW);
  digitalWrite(inB2, HIGH);
  int pwmOutput = map(maxSpeed, 0, maxSpeed, 0 , 255); // Map the potentiometer value from 0 to 5
  analogWrite(enB, pwmOutput); // Send PWM signal to L298N Enable pin
}

void actuatorRight(int speed){
    // Because actuator can not work with less than speed of 5, speed is removed temporarily
  digitalWrite(inB1, HIGH);
  digitalWrite(inB2, LOW);
  int pwmOutput = map(maxSpeed, 0, maxSpeed, 0 , 255); // Map the potentiometer value from 0 to 5
  analogWrite(enB, pwmOutput); // Send PWM signal to L298N Enable pin
}

void actuatorBreakY(){
  digitalWrite(inB1, LOW);
  digitalWrite(inB2, LOW);
  analogWrite(enB, 0); // Send PWM signal to L298N Enable pin
}

void manageActuator(){
  int x = queueManager.getCurrentX();
  int y = queueManager.getCurrentY();

  if(x != 0){
    if(x>0){
      actuatorForward(x);
    }
    else{
      actuatorBackward(-x);
    }
  }
  else{
    actuatorBreakX();
  }
  if(y != 0){
    if(y>0){
      actuatorRight(y);
    }
    else{
      actuatorLeft(-y);
    }
  }
  else{
    actuatorBreakY();
  }
}

void setup() {
  // put your setup code here, to run once:
  // set the data rate for the SoftwareSerial port
  mySerial.begin(115200);

  Serial.begin(9600);
  
  pinMode(enA, OUTPUT);
  pinMode(inA1, OUTPUT);
  pinMode(inA2, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(inB1, OUTPUT);
  pinMode(inB2, OUTPUT);
  

}

void loop() {
  // put your main code here, to run repeatedly:

  //Here the commands are recieved from serial and stored
  manageSerial();

  //Queue manager setup
  queueManager.setStep();

  // Commands are sent to the motor driver
  manageActuator();

}
