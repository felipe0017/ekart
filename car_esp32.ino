/*#define PIN_LRPWM 27
#define PIN_LLPWM 14
#define PIN_RRPWM 12
#define PIN_RLPWM 13

#define PIN_RPOT 35
#define PIN_LPOT 32
#define PIN_DIRPOT 34*/

#define PIN_LRPWM 12
#define PIN_LLPWM 13
#define PIN_RRPWM 27
#define PIN_RLPWM 14

#define PIN_RPOT 32
#define PIN_LPOT 35
#define PIN_DIRPOT 34


int readFiltered(int pin){

  static int buffer_r[8] = {0};
  static int buffer_l[8] = {0};
  static int buffer_d[8] = {0};

  static int idx_r = 0;
  static int idx_l = 0;
  static int idx_d = 0;

  int sum = 0;

  if(pin == PIN_RPOT){

    buffer_r[idx_r] = analogRead(pin);
    idx_r = (idx_r + 1) % 8;

    for(int i=0;i<8;i++) sum += buffer_r[i];
  }

  else if(pin == PIN_LPOT){

    buffer_l[idx_l] = analogRead(pin);
    idx_l = (idx_l + 1) % 8;

    for(int i=0;i<8;i++) sum += buffer_l[i];
  }

  else{

    buffer_d[idx_d] = analogRead(pin);
    idx_d = (idx_d + 1) % 8;

    for(int i=0;i<8;i++) sum += buffer_d[i];
  }

  return sum / 8;
}

void setup(){
  //Serial.begin(115200);

  // 20Khz, 8 bits resolution
  ledcAttach(PIN_LRPWM, 20000, 8);
  ledcAttach(PIN_LLPWM, 20000, 8);
  ledcAttach(PIN_RRPWM, 20000, 8);
  ledcAttach(PIN_RLPWM, 20000, 8);

  // turn off linear actuators
  ledcWrite(PIN_LRPWM, 0);   // forward
  ledcWrite(PIN_LLPWM, 0);   // back
  
  ledcWrite(PIN_RRPWM, 80);  // forward
  ledcWrite(PIN_RLPWM, 0);   // back
}

void loop(){
  
  /*
  - FAILSAFE, LIMITES POR SOFTWARE. AL FINAL UN CHECKEO ANTES DE MANDAR POTENCIA
  */
  
  // read "volante" and feedback of linear actuators (with internal media movile)
  int rpot = readFiltered(PIN_RPOT);
  int lpot = readFiltered(PIN_LPOT);
  int dir_pot = readFiltered(PIN_DIRPOT);

  // put them in the range of [0, 1024]
  lpot = map(lpot, 100, 2200, 0, 1024); // der, izq
  //rpot = map(rpot, 1544, 3995, 0, 1024); // der, izq
  rpot = map(rpot, /*1544*/1895, 3995, 0, 1024); // der, izq
  dir_pot = map(dir_pot, 3330, 510, 0, 1024); // der, izq

  lpot = constrain(lpot, 0, 1024);
  rpot = constrain(rpot, 0, 1024);
  dir_pot = constrain(dir_pot, 0, 1024);

  // compute the error (with sign)
  int l_error = dir_pot - lpot;
  if(abs(l_error) < 30){
    // stop
    ledcWrite(PIN_LRPWM, 0);   // forward
    ledcWrite(PIN_LLPWM, 0);   // back
  }
  else{
    int pwm = constrain(map(abs(l_error), 0, 409, 127, 255), 127, 255);
    //if(pwm > 255) pwm = 255;
    if(l_error < 0){
      // back
      ledcWrite(PIN_LRPWM, 0);   // forward
      ledcWrite(PIN_LLPWM, pwm);   // back
    }
    else{
      // foward
      ledcWrite(PIN_LRPWM, pwm); // forward
      ledcWrite(PIN_LLPWM, 0);   // back
    }
  }

  int r_error = dir_pot - rpot;
  if(abs(r_error) < 30){
    // stop
    ledcWrite(PIN_RRPWM, 0);   // forward
    ledcWrite(PIN_RLPWM, 0);   // back
  }
  else{
    int pwm = constrain(map(abs(r_error), 0, 409, 127, 255), 127, 255);
    //if(pwm > 255) pwm = 255;
    if(r_error < 0){
      // back
      ledcWrite(PIN_RRPWM, pwm);   // back
      ledcWrite(PIN_RLPWM, 0);   // forward
    }
    else{
      // foward
      ledcWrite(PIN_RRPWM, 0); // back
      ledcWrite(PIN_RLPWM, pwm);   // forward
    }
  }
  delay(50); // 20hz
}
