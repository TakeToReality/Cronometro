/*Il programma svolge la funzione di cronometro da corsa (Pulsanti: START/STOP , LAP/RESET)  (Visualizzazione: 6 display 7segmenti)   
Parte premendo START(rosso) , calcola un lap se si preme LAP(nero) , si ferma quando si preme STOP(rosso).
Successivamente vengono visualizzati prima la durata totale della registrazione poi i lap singoli: premendo il pulsante START si possono scorrere,
una volta visualizzati tutti si resetta tutto e riparte.
Il pulsante RESET(nero) può essere usato prima di registrare un tempo per essere sicuri che il programma si azzeri.

Il circuito lavora con display a 7 segmenti comandati serialmente da arduino: per fare ciò si utilizzano dei registri SIPO 74ls164 (Serial Input Parallel Output)
che necessitano di una linea dati dedicata e una reset e una clock (le ultime due sono in parallelo con tutti i dispositivi)
Arduino nella fase di visualizzazione carica in parallelo il dato sulle 6 linee (6 cifre) ed esegue una transizione positiva del clock (fronte di salita)

Nella realtà ho sfruttato un'alimentazione esterna per la breadboard: a causa dell'elevato numero di dispositivi (i led consuma molto perché sono grandi) la corrente
assorbita è consistente e probabilmente il microcontrollore non sarebbe riuscito ad erogarla (la breadboard e arduino hanno la massa in comune per stabilizzare)
*/

#include <stdio.h>

#define dp 0 
#define G 1
#define F 2 
#define E 3
#define D 4
#define C 5
#define B 6
#define A 7

//#define pin_piezo 13  cancellare commenti pinMode e funzioni tone
#define puls_start 2      //D4
#define puls_lap 5        //D1
#define pinclock 4        //D2       in parallelo a tutti i registri
#define pinreset 0        //D3       in parallelo a tutti i registri
#define data1 16          //D0
#define data2 14          //D5
#define data3 12          //D6
#define data4 13          //D7
#define data5 15          //D8
#define data6 3           //RX   

int azione=0;                  //0=il programma non sta facendo nulla    1=sta contando
bool flag_inizia=0;            //se è 1 il è stato premuto start e il programma deve cominciare a contare
bool flag_ferma=0;             //se è 1 il è stato premuto stop e il timer deve essere fermato
bool flag_lap=0;               //""
bool flag_reset=0;             //""
bool lettura_prec;             //flag utile alla lettura del pulsante di start/stop
bool let_prec;                 //flag utile alla lettura del pulsante di lap/reset 
bool flag_prima_inizializzazione=1;           //controlla la prima volta che viene acceso per fare il reset

bool disp[6][8];                        //matrice dei valori (bit), in codice 7 segmenti, di ogni cifra da stampare sui display

void setup() {
  pinMode(puls_start,INPUT_PULLUP);
  pinMode(puls_lap,INPUT_PULLUP);
 // pinMode(pin_piezo,OUTPUT);
  pinMode(pinclock,OUTPUT);
  pinMode(data1,OUTPUT);
  pinMode(data2,OUTPUT);
  pinMode(data3,OUTPUT);
  pinMode(data4,OUTPUT);
  pinMode(data5,OUTPUT);
  pinMode(data6,OUTPUT);
  pinMode(pinreset,OUTPUT);  
  Serial.begin(9600);

  testDisplay();

}



void loop() { 
   static unsigned long t_calcolato;              //intervallo calcolato da mandare a stampa
   const int num_max_lap=10;                      //numero massimo di lap (causa dimensioni array)
   static unsigned long t_lap[num_max_lap+1];     //array di istanti di tempo
   static unsigned long contatore_lap;

//lettura del pulsante rosso (START/STOP) e attivazione degli eventuali flag   controlla che il pulsante sia premuto e rilasciato
    delay(1);
   if(digitalRead(puls_start)==LOW){
    lettura_prec=1;
   }
   else if(lettura_prec==1){
    lettura_prec=0;
    if(azione==0){
      flag_inizia=1;
      azione=1;
    }
    else{
      flag_ferma=1;
      azione=0;
    }
   }

   
//lettura del pulsante nero (LAP/RESET) e attivazione flag  controlla che il pulsante sia premuto e rilasciato
   if(digitalRead(puls_lap)==LOW){
    let_prec=1;
   }
   else if(let_prec==1){
    let_prec=0;
    if(azione==1){
      flag_lap=1;
    }
    else{
      flag_reset=1;
    }
   }

   if(flag_prima_inizializzazione==1){         //la prima volta che il programma viene acceso esegue un reset
    flag_reset=1;
   }


//controllo dei flag e azioni dovute  
   if(flag_inizia==1){                 //START
    t_lap[0]=millis();                 //prende l'istante di tempo inziale (che non è mai 0)
    contatore_lap++;
    beepstart();
    Serial.print("Via-");
    Serial.println(t_lap[0]);
    flag_inizia=0;
   }
   
   else if(flag_ferma==1){                    //STOP
    t_lap[contatore_lap]=millis();          //prende l'istante di tempo finale
    beepstop();
    Serial.print(" stop-");
    Serial.print(t_lap[contatore_lap]);
    Serial.print("[");
    Serial.print(contatore_lap);
    Serial.println("] ");
//sequenza terminata calcola i valori
    t_calcolato=t_lap[contatore_lap]-t_lap[0];      //durata = istanteFinale - istanteIniziale
    stampa(t_calcolato,1);
    for(int i=0; i<contatore_lap; i++){                //calcola le durate dei vari lap
      while(flag_inizia==0){                      //aspetta che il pulsante rosso sia premuto (se non viene premuto rimane bloccato)
        delay(1);
        if(digitalRead(puls_start)==LOW){
          lettura_prec=1;
         }
         else if(lettura_prec==1){
          lettura_prec=0;
          flag_inizia=1;
         }
      }
      flag_inizia=0;
      t_calcolato=t_lap[i+1]-t_lap[i];
      stampa(t_calcolato,1);
    }   
    while(flag_inizia==0){                        //aspetta pressione pulsante rosso (dopo l'ultimo valore si resetta)
      delay(1);
      if(digitalRead(puls_start)==LOW){
        lettura_prec=1;
       }
       else if(lettura_prec==1){
        lettura_prec=0;
        flag_inizia=1;
       }
    }
    flag_inizia=0;
    flag_reset=1;
    flag_ferma=0;
   }  



   else if(flag_reset==1){                //RESET esegue il reset di tutte le variabili           
      flag_reset=0;                             
      t_calcolato=0;
      contatore_lap=0;
      for(int i=0; i<num_max_lap; i++){              //azzeramento array
        t_lap[i]=0;
      }
      for(int i=0; i<6; i++){                     
        for(int j=0; j<8; j++){
          disp[i][j]=0;                       //azzeramento matrice
        }
      }
      beepinizio();
      if(flag_prima_inizializzazione==1){               //solo la prima volta conferma reset iniziale
        Serial.println("Reset iniziale effettuato");
        flag_prima_inizializzazione=0;
      }
      else{
        Serial.println(" reset");
      }
      resetDisplay();
   }
     
   else if(flag_lap==1){
    flag_lap=0;
    if(contatore_lap+1>num_max_lap){
      Serial.print("massimo ");
      Serial.print(num_max_lap);
      Serial.print(" lap ");
    }
    else{
      t_lap[contatore_lap]=millis();              //salva l'istante di tempo nella prima casella vuota del vettore
    }
    Serial.print(" lap-");
    Serial.print(t_lap[contatore_lap]);
    Serial.print(" [");
    Serial.print(contatore_lap);
    Serial.println("]");
    contatore_lap++;
    beeplap();
   }
   
   else{                                              //visualizza in tempo reale sui display il cronometro
    if(azione==1){
      t_lap[contatore_lap]=millis(); 
      t_calcolato=t_lap[contatore_lap]-t_lap[0];      //calcola le durate totali e dei vari lap
      stampa(t_calcolato,0);
    }
   }
}

//utilizzato un beeper / piezo 
void beepinizio(){
 // tone(pin_piezo,1500,100);
}

void beepstart(){
  //tone(pin_piezo,1000,100);
}

void beepstop(){
  //tone(pin_piezo,500,300);
}

void beeplap(){
  //tone(pin_piezo,700,200);
}

void resetDisplay(){                //fornisce un impulso basso sull' ingresso RESET attivo basso
   digitalWrite(pinreset,HIGH);
   digitalWrite(pinreset,LOW);
   digitalWrite(pinreset,HIGH);
}

void testDisplay(){                      //esegue un semplice test (accende in ordine i led)
   resetDisplay();
   Serial.println("test display");
    for(int c=0; c<8; c++){
      digitalWrite(pinclock,LOW);
      digitalWrite(data1,HIGH);
      digitalWrite(data2,HIGH);
      digitalWrite(data3,HIGH);
      digitalWrite(data4,HIGH);
      digitalWrite(data5,HIGH);
      digitalWrite(data6,HIGH);
      digitalWrite(pinclock,HIGH); 
      delay(100);
  }
   delay(100);
   resetDisplay();
}

void stampa(unsigned long input,int flag_stampa){            //trasforma da millisecondi a minuti:secondi,centesimi li connverte in 7segm e stampa
  int centesimi;                                              //flag stampa 0=non stampa sul monitor serial, 1=stampa, 2=per stampa in tempo reale
  int secondi;                                                
  int minuti;
  int valore[6];                          //vettore delle cifre del valore
  bool salta_if=0;        //flag per stampa in tempo reale
    
  centesimi=input%1000;                   //prima calcola i valori      
  centesimi=centesimi/10;      //abbassa la precisione a due decimali
  secondi=input/1000;
  minuti=secondi/60;                
  secondi=secondi%60;
  
  if(minuti>=60){
    Serial.println("Errore: numero massimo rappresentabile raggiunto");
  }

  valore[0]=minuti/10;              //poi separe le due cifre per ogni nelle due caselle così da lavorarci singolarmente
  valore[1]=minuti&10;
  valore[2]=secondi/10;
  valore[3]=secondi%10;
  valore[4]=centesimi/10;
  valore[5]=centesimi%10;
  
  if(flag_stampa==1){                                      //stampa sul monitor seriale
    Serial.print(valore[0]);         //minuti    
    Serial.print(valore[1]);
    Serial.print(":");
    Serial.print(valore[2]);        //secondi
    Serial.print(valore[3]);
    Serial.print(",");
    Serial.print(valore[4]);          //centesimi
    Serial.println(valore[5]);
  }

  if(flag_stampa==2){
    for(int i=0; i<5; i++){
      if(valore[i]==0 && salta_if==0){
        for(int j=0; j<8; j++){
          disp[i][j]=0;
        }
      }
      else{
        decto7seg(valore[i],i);
        salta_if=1;
      }
    }
  }
  else{
    for(int i=0; i<6; i++){
      decto7seg(valore[i],i);
    }
  }
  
  
  for(int i=0; i<8; i++){               //stampa su display 7 seg
    digitalWrite(pinclock,LOW);           //clock basso
    digitalWrite(data1,disp[0][i]);
    digitalWrite(data2,disp[1][i]);
    digitalWrite(data3,disp[2][i]);       //dati
    digitalWrite(data4,disp[3][i]);
    digitalWrite(data5,disp[4][i]);
  //  digitalWrite(data6,disp[5][i]);
    digitalWrite(pinclock,HIGH);        //fronte positivo clock
  }
}

void decto7seg(int input, int indice){                //trasforma da decimale a codice per display a 7 segmenti
  switch(input){
    case 0:                     //numero decimale 0
      disp[indice][dp]=0;            
      disp[indice][G]=0;            //codifica fatta da me per ogni numero
      disp[indice][F]=1;
      disp[indice][E]=1;          //  _
      disp[indice][D]=1;         //  | |
      disp[indice][C]=1;         //  |_|
      disp[indice][B]=1;
      disp[indice][A]=1;
    break;
    case 1:
      disp[indice][dp]=0;
      disp[indice][G]=0;
      disp[indice][F]=0;
      disp[indice][E]=0;        //  |
      disp[indice][D]=0;        //  |
      disp[indice][C]=1;
      disp[indice][B]=1;
      disp[indice][A]=0;
    break;
    case 2:
      disp[indice][dp]=0;
      disp[indice][G]=1;
      disp[indice][F]=0;      //  _
      disp[indice][E]=1;      //  _|
      disp[indice][D]=1;     //  |_
      disp[indice][C]=0;
      disp[indice][B]=1;
      disp[indice][A]=1;
    break;
    case 3:
      disp[indice][dp]=0;
      disp[indice][G]=1;
      disp[indice][F]=0;      //  _
      disp[indice][E]=0;      //  _|
      disp[indice][D]=1;     //   _|
      disp[indice][C]=1;
      disp[indice][B]=1;
      disp[indice][A]=1;
    break;
    case 4:
      disp[indice][dp]=0;
      disp[indice][G]=1;
      disp[indice][F]=1;        // |_|
      disp[indice][E]=0;      //     |
      disp[indice][D]=0;
      disp[indice][C]=1;
      disp[indice][B]=1;
      disp[indice][A]=0;
    break;
    case 5:
      disp[indice][dp]=0;
      disp[indice][G]=1;
      disp[indice][F]=1;
      disp[indice][E]=0;       //   _
      disp[indice][D]=1;    //     |_
      disp[indice][C]=1;       //   _|
      disp[indice][B]=0;
      disp[indice][A]=1;
    break;
    case 6:
      disp[indice][dp]=0;
      disp[indice][G]=1;
      disp[indice][F]=1;
      disp[indice][E]=1;          //  _
      disp[indice][D]=1;          // |_
      disp[indice][C]=1;         //  |_|
      disp[indice][B]=0;
      disp[indice][A]=1;
    break;
    case 7:
      disp[indice][dp]=0;
      disp[indice][G]=0;
      disp[indice][F]=0;
      disp[indice][E]=0;        //  _
      disp[indice][D]=0;        //   |
      disp[indice][C]=1;      //     |
      disp[indice][B]=1;
      disp[indice][A]=1;
    break;
    case 8:
      disp[indice][dp]=0;
      disp[indice][G]=1;
      disp[indice][F]=1;
      disp[indice][E]=1;      //  _
      disp[indice][D]=1;      // |_|
      disp[indice][C]=1;    //   |_|
      disp[indice][B]=1;
      disp[indice][A]=1;
    break;
    case 9:
      disp[indice][dp]=0;
      disp[indice][G]=1;  
      disp[indice][F]=1;      //  _
      disp[indice][E]=0;      // |_|
      disp[indice][D]=1;      //  _|
      disp[indice][C]=1;
      disp[indice][B]=1;
      disp[indice][A]=1;
    break;
  }

  if(indice%2==1){               //ogni due cifre, abilita la virgola del diplay come separatore
    disp[indice][dp]=1;
  }

}
