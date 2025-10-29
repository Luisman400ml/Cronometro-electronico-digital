/////////////////////////////////////////////////////////////////////////Programa de Luis Manuel Gonzalo; creado en 2023-2024///////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
//////Resistecnia de Pull-down para controlar la tensión de la fotocelula
/////////////////////////
///////////////////////////////Librerias y constantes//////////////////////////////////////////////////
//#include <SoftwareSerial.h>
#include <math.h>
#include <esp_now.h>
#include <WiFi.h>

#define constFoto 1022


//////////////////////////////Declaración de variables://///////////////////////////////////////////
int start=1;
int game_mode=0;

/*int dist_1=0;
int dist_2=0;
int dist_3=0;
int dist_4=0;
*/
int dist_total=0;
int dists[4]={0};

float vel_1=0;
float vel_2=0;
float vel_3=0;
float vel_4=0;

//pendientes variables de aceleración//

float time_1=0;
float time_2=0;
float time_3=0;
float time_4=0;
float time_total=0;

int milis=0;
int segundos=0;
int minutos=0;
int horas=0;
int contador=0;

float tramo_1=0;           ///////////para tiempos de cada tramo
float tramo_2=0;
float tramo_3=0;

unsigned long contador10ms=0;

int num_tokens=0;                  /////////////////para activar la fotocelula correspondiente
int token1_activo=1;
int token2_activo=0;
int token3_activo=0;
int token4_activo=0;
int token5_activo=0;
int tokenV_activo=1;
int token_espera=0;

String msg1="Numero de fotocelulas a activar (recuerda que has de activar de 2 a 5 fotocelulas, en caso contrario habra errores): ";
String msg2="¿Distancia igual entre fotocelulas? [si=1 no=0] ";
String msg3="Distancia entre las fotocelulas (en metros): ";
String msg4=" fotocelulas seran activadas.";
String msg5="Distancia entre las fotocelulas (en metros): ";
String msg6="Selecciona modo de juego: [0=series] [1=salidas] [2=vuelta a vuelta] ";
String msg7="Listo [1=ready]";
String msg8="Para este formato se usa solo la fotocelula 1.";
String msg9="Tiempo de vuelta ";
String msg10="Inicio de serie";
String msg11="Tiempo total: ";

int lectura=0;
uint8_t num_foto=0;
int same_dist=0;

int on_marks=0;
int shoot=0;
int lap_1=1;
int laps_counter=1;
int warning=1;

int i;

int fin_crono=0;

int mensaje=0;
uint8_t habilitar=1;
/////////////////////////////////////////Para formato de MAC
void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
// Formats MAC Address
{
  snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

/////////////////////////////////configuración del arduino

hw_timer_t *timer = NULL;

/////////////////////////Función del TIMER0
void IRAM_ATTR timerInterrupcion() {
 contador10ms++;
}

void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
// Called when data is received
{
  // Only allow a maximum of 250 characters in the message + a null terminating byte
  char buffer[ESP_NOW_MAX_DATA_LEN + 1];
  int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
  strncpy(buffer, (const char *)data, msgLen);
 
  // Make sure we are null terminated
  buffer[msgLen] = 0;
 
  // Format the MAC address
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
 
  // Send Debug log message to the serial port
  Serial.printf("Received message from: %s - %s\n", macStr, buffer);
 
  // Check switch status
  if (strcmp("11", buffer) == 0){
   mensaje=11; 
  } else if(strcmp("22", buffer) == 0){
   mensaje=22;
  }else if(strcmp("33", buffer) == 0){
   mensaje=33;
  }else if(strcmp("44", buffer) == 0){
   mensaje=44;
  }else if(strcmp("55", buffer) == 0){
   mensaje=55;
  }
  }
 
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status){
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  Serial.print("Last Packet Sent to: ");
  Serial.println(macStr);
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

///////////////////////////////////////////////Para configuracion inalambrica de los nodos
void broadcast(const String &message){
  
  // NODO 1
  uint8_t broadcastAddress1[] = {0x24, 0x4C, 0xAB, 0x82, 0xF1, 0x68};
  uint8_t broadcastAddress2[] = {0xE4, 0x65, 0xB8, 0x77, 0x5D, 0xBC};
  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, broadcastAddress1, 6);
  if (!esp_now_is_peer_exist(broadcastAddress1))
  {
    esp_now_add_peer(&peerInfo);
  }
  // Send message
  esp_err_t result = esp_now_send(broadcastAddress1, (const uint8_t *)message.c_str(), message.length());
  //---------------------------------------------
  memcpy(&peerInfo.peer_addr, broadcastAddress2, 6);
  if (!esp_now_is_peer_exist(broadcastAddress2))
  {
    esp_now_add_peer(&peerInfo);
  }
  // Send message
  esp_err_t result2 = esp_now_send(broadcastAddress2, (const uint8_t *)message.c_str(), message.length());
}

 
/////////////////////////////////////////////////////////////////////////////////////////////////



void setup() {
  Serial.begin(9600);
  //Contador para Arduino
  /*
  TCCR1A = 0;                // El registro de control A queda todo en 0
  TCCR1B = 0;                //limpia registrador
  TCNT1  = 0;                //Inicializa el temporizador
  OCR1A = 0x270;            // carga el registrador de comparación: 16MHz/1024/1Hz -1 = 15624 = 0X3D08
  TCCR1B |= (1 << WGM12)| (1 << CS12);   // modo CTC, prescaler de 1024: CS12 = 1 e CS10 = 1  
  TIMSK1 |= (1 << OCIE1A);  // habilita interrupción por ig
  */

//Configuracion timer esp32
timer = timerBegin(0, 80, true); // Timer 0, divisor de reloj 80
  timerAttachInterrupt(timer, &timerInterrupcion, true); // Adjuntar la función de manejo de interrupción
  timerAlarmWrite(timer, 10000, true); // Interrupción cada 10 milisegundos?
  timerAlarmEnable(timer); // Habilitar la alarma

  ///////////Configuracion de la conexion
  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA); //////////////////modo ap, mejor?
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK)
  {
    Serial.println("ESP-NOW Init Success");
    esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
  }
  else
  {
    Serial.println("ESP-NOW Init Failed");
    delay(3000);
    ESP.restart();
  }
  
  
  // Register callback function
  esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
}


void loop() {
configuracion();
/////////////////////////////////////////////////////////////////////////////////////////TOMA DE TIEMPOS/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if((token1_activo==1)&&(game_mode==0)){                         /////Lectura de la primera fotocelula por pin A0   modo 0
    
    if(mensaje==11){               /////////si pin activo entramos en función
    Serial.println();
    Serial.println(msg10);
    Serial.println();
    contador10ms=0;                             //////Iniciamos el contador a 0
    token1_activo=0;
    token2_activo=1;
    }
  
  }
  if((token1_activo==1)&&(game_mode==1)){                         /////Lectura de la primera fotocelula por pin A0   modo 1
    if((mensaje==11)&&(on_marks==0)){               /////////si pin activo entramos en función
    //on_marks=1;
    Serial.println(msg7);
    while(Serial.available()==0){}
      shoot=Serial.parseInt();
      Serial.println("A la espera de la salida (1) (Este programa no se hace responsable de los raspones en las zapatillas)");
      on_marks=1;
    }
      Serial.read();

    if((mensaje==11)&&(shoot==1)&&(on_marks==1)){
      contador10ms=0;
      Serial.println("Salida detectada");
      token1_activo=0;
      token2_activo=1;
      on_marks=0;
      shoot=0;
    }
    
    
  
  }
  
  if(token2_activo==1){                         /////Lectura de la primera fotocelula por pin A1
    analogRead(1);                              /////creo que esto sobra
    if(mensaje==22){               /////////si pin activo entramos en función
    time_1=contador10ms;                          //////Iniciamos el contador a 0
    token2_activo=0;
    if(num_foto>2){
    token3_activo=1;
    }
    if(num_foto==2){
    fin_crono=1;
    token1_activo=1;
    }
    
    Serial.println();
    time_1=time_1/100;                              ///////////Tiempo a segundos
    vel_1=dists[0]/time_1;                          ///////////velocidad
    Serial.print("Tiempo de paso en el primer parcial: ");     
    Serial.println(time_1);
    Serial.print("Velocidad media en el primer parcial: ");
    Serial.println(vel_1);
    Serial.println();
  }
  }

  if((token3_activo==1)&&(num_foto>=3)){                         /////Lectura de la primera fotocelula por pin A2
    analogRead(2);                              /////creo que esto sobra
    if(mensaje==33){               /////////si pin activo entramos en función
    time_2=contador10ms;                          //////Iniciamos el contador a 0
    //Serial.print(time_2);
    tramo_1=time_2-time_1;
    token3_activo=0;
    if(num_foto>3){
    token4_activo=1;
    }
    if(num_foto==3){
    fin_crono=1;
    token1_activo=1;
    }

    time_2=time_2/100;                              ///////////Tiempo a segundos
    tramo_1=time_2-time_1;
    vel_2=dists[1]/tramo_1;                          ///////////velocidad
    Serial.print("Tiempo de paso en el segundo parcial: ");     
    Serial.println(time_2);
    Serial.print("Velocidad media en el segundo parcial: ");
    Serial.println(vel_2);
    Serial.print("Tiempo del tramo: ");
    Serial.println(tramo_1);
    Serial.println();
    }
  
  }
  
  if((token4_activo==1)&&(num_foto>=4)){                         /////Lectura de la primera fotocelula por pin A3
    analogRead(3);                              /////creo que esto sobra
    if(mensaje==44){               /////////si pin activo entramos en función
    time_3=contador10ms;                          //////Iniciamos el contador a 0
    //Serial.print(time_3);
    tramo_2=time_3-time_2;
    token4_activo=0;
    if(num_foto>4){
    token5_activo=1;
    }
    if(num_foto==4){
    fin_crono=1;
    token1_activo=1;
     }

    time_3=time_3/100;                              ///////////Tiempo a segundos
    tramo_2=time_3-time_2;
    vel_3=dists[2]/tramo_2;                          ///////////velocidad
    Serial.print("Tiempo de paso en el tercer parcial: ");     
    Serial.println(time_3);
    Serial.print("Velocidad media en el tercer parcial: ");
    Serial.println(vel_3);
    Serial.print("Tiempo del tramo: ");
    Serial.println(tramo_2);
    Serial.println();
    }
    }
  
  

  if((token5_activo==1)&&(num_foto>=5)){                         /////Lectura de la primera fotocelula por pin A4
    analogRead(4);                              /////creo que esto sobra
    if(mensaje==55){               /////////si pin activo entramos en función
    time_4=contador10ms;                          //////Iniciamos el contador a 0
    //Serial.print(time_4);
    tramo_3=time_4-time_3;
    token5_activo=0;
    token1_activo=1;
    fin_crono=1;

    time_4=time_4/100;                              ///////////Tiempo a segundos
    tramo_3=time_4-time_3;
    vel_4=dists[3]/tramo_3;                          ///////////velocidad
    Serial.print("Tiempo de paso en el cuarto parcial: ");     
    Serial.println(time_4);
    Serial.print("Velocidad media en el cuarto parcial: ");
    Serial.println(vel_4);
    Serial.print("Tiempo del tramo: ");
    Serial.println(tramo_3);
    Serial.println();

    }
  
  }
/////////////////////////////////////////////////////REVISAR RESTO DE IMPLEMENTACION HACIA ABAJO
  if(game_mode==2){                ////////////////////////////////modo vuelta a vuelta     
   if(warning==1){
    Serial.println(msg8);
    Serial.println();
    
    warning=0;
   }
   if((analogRead(0)>=constFoto)&&(lap_1==1)&&(tokenV_activo==1)){       /////////Primer paso por la fotocelula
   contador10ms=0;
   lap_1=0;
   tokenV_activo=0;
   
  }
  if(analogRead(0)<constFoto){
    tokenV_activo=1;
  }
  if((analogRead(0)>constFoto)&&(lap_1==0)&&(tokenV_activo==1)){         ///////////Proceso vuelta a vuelta
  time_1=contador10ms;
  time_1=time_1/100;
  Serial.print(msg9);                                          ////////////////Tiempo de vuelta 
  Serial.print(laps_counter);
  Serial.print(": ");
  Serial.println(time_1-time_2);
  Serial.print(msg11);                                ///////////////////Tiempo total
  calculo_tiempo();
  Serial.println();
  laps_counter++;
  time_2=time_1;
  tokenV_activo=0;

  } 
  }
  if(fin_crono==1){

  
  Serial.println("Fin de serie");
  Serial.println();
  Serial.println("Resumen:");
  Serial.print("Tiempo final: ");
  calculo_tiempo();
  Serial.println();
  Serial.print("Distancia:");


   

  fin_crono=0;
  }
}

//Funcion contador arduino
/*
ISR(TIMER1_COMPA_vect){                      ///////////////////Función del contador
  contador10ms++;

}*/
///////////////////////////////////////////////////////////Función para el calculo e impresión bonita del tiempo//////////////////////////////
void calculo_tiempo(){
  milis=contador10ms;
  if(contador10ms>=100){
    segundos=(contador10ms/100);
    milis=contador10ms-(segundos*100);
  }
  if((segundos/60)>=1){
  minutos=(segundos/60);
  segundos=(segundos%60);
  }
  if(minutos>=60){
   horas=(minutos/60);
   minutos=(minutos%60);
  }
  if(horas>=1){
   Serial.print(horas);
  Serial.print("h ");
  }
  if(minutos>=1){
  Serial.print(minutos);
  Serial.print("m ");
  }
  if(segundos>=1){
  Serial.print(segundos);
  Serial.print(".");
}
if(segundos==0){
  Serial.print(0);
  Serial.print(".");

}
if(milis<10){
  Serial.print(0);
  Serial.print(milis);
  Serial.print("s");
  Serial.println();
}
if(milis>=10){
  Serial.print(milis);
  Serial.print("s");
  Serial.println();
}
}

////////////////////////////////////////////////////////////////////////////PETICION DE DATOS AL USUARIO : //////////////////////////////////////////////////////////////////////////////////////////
void configuracion(){
if(start==1){
  Serial.println(msg6);
  while(Serial.available()==0){}
  game_mode=Serial.parseInt();

  Serial.read();

 if((game_mode==0)||(game_mode==1)){
 Serial.println(msg1);                                     /////////////Pregunto por las fotocelulas a activar
 while(Serial.available()==0){}                            /////////////Espero al dato
 num_foto=Serial.parseInt();
 Serial.print(num_foto);                                    /////////////Informo del valor aportado
 Serial.println(msg4);

 Serial.read();                                             /////////////Limpiamos buffer
 
 if(num_foto==2){                                            ////////////////Pido distancia si solo 2 fotocelulas activadas
 Serial.println(msg3);
 while(Serial.available()==0){}
 dists[0]=Serial.parseInt();
 //Serial.print(dist_1);
 }else{

 Serial.println(msg2);                                     ///////////////Pregunto si la distancia es igual en todos los tramos si hay mas de 2 fotocelulas activadas
 while(Serial.available()==0){}
 same_dist=Serial.parseInt();
 if(same_dist==0)  
 Serial.println("Distancias distintas.");                            ///////////////Informo del valor aportado
 if(same_dist==1)                                
 Serial.println("Distancias iguales.");

 Serial.read();                                             /////////////Limpiamos buffer

 if(same_dist==1){                                          ////////////////////////Pregunto la distancia entre fotocelulas si es igual
 Serial.println(msg3);
 while(Serial.available()==0){}
 dists[0]=Serial.parseInt();
 dists[1]=dists[0];
 dists[2]=dists[0];
 dists[3]=dists[0];
 }

 if(same_dist==0){                                            ////////////////////////////Si las distacias son distintas, pido cada una de ellas.
 for(i=2;i<=num_foto;i++){
 Serial.print(msg5);
 Serial.print(i-1);
 Serial.print(" y ");
 Serial.println(i);
 while(Serial.available()==0){}
 dists[i-2]=Serial.parseInt();                                   ///////////////////////Guardo cada distancia

 Serial.read();                                             /////////////Limpiamos buffer
 //Serial.println[i-2];

 }
 
 }

 
 }
 }

 Serial.read();

 Serial.println("Configuración completa");

 start=0;
 ///////////////////////////////////Activacion via inalambrica de las fotocelulas exclavas
 if(num_foto==2){
 broadcast("2");
 } else if(num_foto==3){
 broadcast("3");
}  else if(num_foto==4){
 broadcast("4");
}  else if(num_foto==5){
 broadcast("5");
}




}


//////////////////////////////////////////////////////////////////////////////////////Fin petición de datos al usuario////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}