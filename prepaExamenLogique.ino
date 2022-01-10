// flanc montant
class PositivEdge {
  private :
    boolean memPrevState;
    boolean out;
  public :
    PositivEdge(boolean condition);                      //constructor
    boolean eval(boolean condition);
    boolean get_out();
};
PositivEdge::PositivEdge(boolean condition) {
  this->memPrevState = condition;
}
boolean PositivEdge::eval(boolean condition) { //update positiv edge state must be done ONLY ONCE by loop cycle
  out = condition && !memPrevState;
  memPrevState = condition;
  return out;
}
boolean PositivEdge::get_out() {  //use get_out() to know positiv edge state (use more than once by cycle is possible)
  return this < - out;
} // fin flanc montant

class OnDelayTimer {

  private :
    unsigned long presetTime = 1000;
    unsigned long memStartTimer = 0;            //memory top timer activation
    unsigned long elpasedTime = 0;             //elapsed time from start timer activation
    boolean memStartActivation;                //for positive edge detection of the activation condition
    boolean outTimer;                          //timer's out : like normally open timer switch
  public :
    OnDelayTimer(unsigned long _presetTime);   //constructor
    boolean updtTimer(boolean activation);      //return tempo done must be executed on each program scan
    unsigned long get_elapsedTime();           //return
    set_presetTime(unsigned long _presetTime); //change defaut preset assigned when instance created
    boolean get_outTimer();

};//end class OnDelayTimer
//constructor
OnDelayTimer::OnDelayTimer(unsigned long presetTime) {
  this -> presetTime = presetTime;
}
boolean OnDelayTimer::updtTimer(boolean activation) {
  if (!activation) {
    elpasedTime = 0;
    memStartActivation = false;
    outTimer = false;
    return false;
  } else {

    if (!memStartActivation) {
      memStartTimer = millis();
      memStartActivation = true;
    }
    elpasedTime = millis() - memStartTimer;
    outTimer = elpasedTime >= this->presetTime; //update timer 's "switch"
    return  outTimer;

  }
}//end endTimer()
//constructor
boolean OnDelayTimer::get_outTimer() {

  return this->outTimer;
}

// pin de sortie 22->27

const int iPIN_H1 = 22;
const int iPIN_H2 = 23;
const int iPIN_H3 = 24;
const int iPIN_presse = 25;
const int iPIN_inject = 26;
const int iPIN_rotation = 27;
const int iPIN_chauffe = 28;
const int iPIN_Ev_inject = 29;

//pin entrée NO 40->46
const int iPIN_Bp_start = 40;
const int iPIN_Bp_reset = 41;
const int iPIN_presse_in = 42;
const int iPIN_presse_out = 43;
const int iPIN_inject_in = 44;
const int iPIN_inject_out = 45;

// pin entrée NF 47->49
const int iPIN_Bp_stop = 47;
const int iPIN_AU = 48;
const int iPIN_pressionOK = 49;

boolean H1, H2, H3, presse, inject, rotation, chauffe, Ev_inject = 0; //boolean sortie
boolean Bp_start, Bp_reset, presse_in, presse_out, inject_in, inject_out, Bp_stop, AU, pressionOK = 0; //boolean entrée
//variable additionel
boolean Stop, reset, defaut = 0;

// nombre étapes et transistions
const unsigned int nbStepPr = 6;
const unsigned int nbTransition = 7;
boolean stepPr[nbStepPr];
boolean transition[nbTransition];
boolean stepAU[2];
boolean transitionAU[2];
boolean stepLed[2];
boolean transitionLed[2];

//déclaration débug
String strDebugLine;
int stp, stpAU, stpLed;// étape numéro x dans le debug

// déclaration des flanc montant à getter: PositivEdge nomDeVariable(nomDeVariable à évaluer)
PositivEdge posEdge_Bp_start(Bp_start);
PositivEdge posEdge_Bp_reset(Bp_reset);
// déclaration timer : OnDelayTimer nomDeVariable(temps en milliseconde);
OnDelayTimer timerstepPr4(2000);
OnDelayTimer timerstepLed0(1000);
OnDelayTimer timerstepLed1(1000);

void setup() {
  Serial.begin(9600); // déclaration moniteur série
  // sortie
  pinMode(iPIN_H1, OUTPUT);
  pinMode(iPIN_H2, OUTPUT);
  pinMode(iPIN_H3, OUTPUT);
  pinMode(iPIN_presse, OUTPUT);
  pinMode(iPIN_inject, OUTPUT);
  pinMode(iPIN_rotation, OUTPUT);
  pinMode(iPIN_chauffe, OUTPUT);
  pinMode(iPIN_Ev_inject, OUTPUT);
  //entrée
  pinMode(iPIN_Bp_start, INPUT);
  pinMode(iPIN_Bp_reset, INPUT);
  pinMode(iPIN_presse_in, INPUT);
  pinMode(iPIN_presse_out, INPUT);
  pinMode(iPIN_inject_in, INPUT);
  pinMode(iPIN_inject_out, INPUT);
  pinMode(iPIN_pressionOK, INPUT);

  pinMode(iPIN_Bp_stop, INPUT);
  pinMode(iPIN_AU, INPUT);

  stepPr[0] = true; // début stepPr
  stepAU[0] = true;
  stepLed[0] = true;
}

void loop() {
  //lecture entrée
  Bp_start = digitalRead (iPIN_Bp_start);
  Bp_reset = digitalRead (iPIN_Bp_reset);
  presse_in = digitalRead (iPIN_presse_in);
  presse_out = digitalRead (iPIN_presse_out);
  inject_in = digitalRead (iPIN_inject_in);
  inject_out = digitalRead (iPIN_inject_out);
  Bp_stop = digitalRead (iPIN_Bp_stop);
  AU = digitalRead (iPIN_AU);
  pressionOK = digitalRead (iPIN_pressionOK);
  defaut = AU && pressionOK;


  //evaluation flanc montant: posEdge_nomDeVariable.eval(nomDeVariable)
  posEdge_Bp_start.eval(Bp_start);
  posEdge_Bp_reset.eval(Bp_reset);

  // déclaration des transitions
  transition[0] = stepPr[0] && posEdge_Bp_start.get_out() && Bp_stop && AU;
  transition[1] = stepPr[1] && presse_in && inject_in;
  transition[2] = stepPr[2] && presse_out;
  transition[3] = stepPr[3] && inject_out;
  transition[4] = stepPr[4] && timerstepPr4.get_outTimer();
  transition[5] = stepPr[5] && presse_in && inject_in && !Stop;
  transition[6] = stepPr[5] && presse_in && inject_in && Stop;


  // stepPr non linéraire
  if (transition[0]) {
    stepPr[0] = false;
    stepPr[1] = true;
  }
  if (transition[1]) {
    stepPr[1] = false;
    stepPr[2] = true;
  }
  if (transition[2]) {
    stepPr[2] = false;
    stepPr[3] = true;
  }
  if (transition[3]) {
    stepPr[3] = false;
    stepPr[4] = true;
  }
  if (transition[4]) {
    stepPr[4] = false;
    stepPr[5] = true;
  }
  if (transition[5]) {
    stepPr[5] = false;
    stepPr[2] = true;
  }
  if (transition[6]) {
    stepPr[5] = false;
    stepPr[0] = true;
  }

  Stop = (Stop || !Bp_stop) && !stepPr[0];

  // G7 AU
  transitionAU[0] = stepAU[0] && !defaut;
  transitionAU[1] = stepAU[1] && defaut && !reset;

  if (transitionAU[0]) {
    stepAU[0] = false;
    stepAU[1] = true;
  }
  if (transitionAU[1]) {
    stepAU[1] = false;
    stepAU[0] = true;
  }

  reset = (reset || stepAU[0]) && !posEdge_Bp_reset.get_out();

  //G7 led
  transitionLed[0] = stepLed[0] && timerstepLed0.get_outTimer() && reset;
  transitionLed[1] = stepLed[1] && timerstepLed1.get_outTimer();

  if (transitionLed[0]) {
    stepLed[0] = false;
    stepLed[1] = true;
  }
  if (transitionLed[1]) {
    stepLed[1] = false;
    stepLed[0] = true;
  }

  //déclaration code
  if (stepAU[1]) {
    stepPr [0] = true;
    for (int i = 1; i < nbStepPr; i++) {
      stepPr [i] = false;
    }
  }


  //sortie activée par Step (sortie = stepPr[x])
  H1 = !stepPr[0];
  H2 = stepLed[0] && stepAU[1];
  H3 = stepPr[0] && stepAU[0];
  presse = stepPr[2] || stepPr[3] || stepPr[4];
  inject = stepPr[3] || stepPr[4];
  rotation = stepPr[2] || stepPr[3] || stepPr[4] || stepPr[5];
  chauffe = stepPr[2] || stepPr[3] || stepPr[4] || stepPr[5];
  Ev_inject = stepPr[4] && presse_out;

  // timer update (s'active à l'étape x)
  timerstepPr4.updtTimer(stepPr[4]);
  timerstepLed0.updtTimer(stepLed[0]);
  timerstepLed1.updtTimer(stepLed[1]);

  //association Sortie-Pin
  digitalWrite (iPIN_H1, H1);
  digitalWrite (iPIN_H2, H2);
  digitalWrite (iPIN_H3, H3);
  digitalWrite (iPIN_presse, presse);
  digitalWrite (iPIN_inject, inject);
  digitalWrite (iPIN_rotation, rotation);
  digitalWrite (iPIN_chauffe, chauffe);
  digitalWrite (iPIN_Ev_inject, Ev_inject);

  //debug: étapes active
  for (int i = 0; i < nbStepPr; i++) {
    if (stepPr[i]) {
      stp = i;
      break;
    }
  }
  for (int i = 0; i < 2; i++) {
    if (stepAU[i]) {
      stpAU = i;
      break;
    }
  } for (int i = 0; i < 2; i++) {
    if (stepLed[i]) {
      stpLed = i;
      break;
    }
  }

  // sortie debug
  strDebugLine = "stepPr:" + String(stp, DEC) + " stepAU:" + String(stpAU, DEC) + " stepLed:" + String(stpLed, DEC) +
                 " Bp_start:" + String(Bp_start, DEC) + " Bp_reset:" + String(Bp_reset, DEC) + " presse_in:" + String(presse_in, DEC) + " presse_out:" + String(presse_out, DEC) + " inject_in:" + String(inject_in, DEC) +  " inject_out:" + String(inject_out, DEC) +
                 " Bp_stop:" + String(Bp_stop, DEC) + " AU:" + String(AU, DEC) +
                 " H1:" + String(H1, DEC) + " H2:" + String(H2, DEC) + " H3:" + String(H3, DEC) + " presse:" + String(presse, DEC) + " inject:" + String(inject, DEC) + " rotation:" + String(rotation, DEC) + " chauffe:" + String(chauffe, DEC) + " Ev_inject:" + String(Ev_inject, DEC);
  Serial.println(strDebugLine);
}
