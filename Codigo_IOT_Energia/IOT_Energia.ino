// Importação das Bibliotecas
#include <ESP8266WiFi.h>  // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient do MQTT
#include <NTPClient.h>    // Importa a Biblioteca NTPClient para conexão com servidor NTP para coletar o horario em formato Epoch
#include <WiFiUdp.h>      // Importa a Biblioteca WiFiUdp para permitir enviar e receber pacotes via UDP
#include <Time.h>         // Complemento da Biblioteca TimeLib.h
#include <TimeLib.h>      // Importa Biblioteca para realizar a conversão do formato Epoch para ISO no metodo epochToLabIoTFormat

// MQTT
#define TOPICO_PUBLISH "consumo_Eletrico_LabIoT"
#define ID_MQTT  "id_consumo_Eletrico_LabIoT_001"
const char* BROKER_MQTT = ""; //URL ou IP do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT normalmente a 1883
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

// Definições de conexão na rede
const char* SSID = "LabIoT"; // SSID nome da rede WI-FI que deseja se conectar
const char* PASSWORD = ""; // Senha da rede WI-FI que deseja se conectar

// Configurações para coletar Horario via internet
WiFiUDP ntpUDP;//biblioteca para envio de mensagem via protocolo UDP
int16_t utc = -3; //UTC -3:00 Brazil
NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utc * 3600, 60000);// NTP get Time - Configurações para consulta do horario atual via internet

// Constantes Globais para calculo do consumo
float mediaCorrente = 0;
int tensaoRedeEletrica = 127; // 127 ou ou 220 dependendo da tensão da tomada aonde for ser realizado a medição

// Portas dos entrada do Sensor
#define pinoSensor_ACS712_5A A0

// Constantes Globais utilizadas no loop
String mensagem = "";
unsigned int horario = 0;

void setup() {
  //Incia a Serial
  Serial.begin(9600);
  pinMode(pinoSensor_ACS712_5A, INPUT);
  reconectWiFi(); // conecta-se a rede WIFI
  initMQTT(); // conecta-se ao broker MQTT
}

void loop() {
  
  mensagem.remove(0); //remove todo o conteudo da string a partir da posição passada por parametro (da esquerda para a direita)
  mensagem.concat(getSensor());//getSensor pega a media já calculada do consumo no intervalo de um segundo entre os picos de onda
  mensagem.concat(' ');
  mensagem.concat(epochToISO(timeClient.getEpochTime()));//timeClient.getEpochTime() pega o horario em formato epoch na internet para adicionar na mensagem
                                                         //epochToISO() faz a conversão do formato epoch para o formato ISO utilizado no banco de dados do servidor MQTT LabIoT
  Serial.println(mensagem.c_str());
  verificaConexoesWifiAndMQTT();
  enviaEstadoOutputMQTT();
  delay(58200); //Somado ao tempo de leitura enviara uma mensagem aproximadamente a cada 1 minuto
}

/*
   Função: Converter horario em formato epoch para ISO
   Parâmetros: horario em formato epoch unsigned int
   Retorno: retorna uma String com o horario epoch convertido para formato ISO
*/
String epochToISO(unsigned int horario){
  
  String mensagem = "";
  if(year(horario) < 10){
    mensagem.concat("0");
  }
  mensagem.concat((String) year(horario) + "-");
  
  if(month(horario) < 10){
    mensagem.concat("0");
  }
  mensagem.concat((String) month(horario) + "-");

  if(day(horario) < 10){
    mensagem.concat("0");
  }
  mensagem.concat(String(day(horario)) + "T");

  if(hour(horario) < 10){
    mensagem.concat("0");
  }
  mensagem.concat((String) hour(horario) + ":");

  if(minute(horario) < 10){
    mensagem.concat("0");
  }
  mensagem.concat((String) minute(horario) + ":");

  if(second(horario) < 10){
    mensagem.concat("0");
  }
  mensagem.concat((String) second(horario) + "Z");

  return mensagem;
 }

/*
   Função: Inicializa MQTT e informa qual broker e porta deve ser conectado
   Parâmetros: nenhum
   Retorno: nenhum
*/
void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
}

/*
   Função: Verifica conexçoes WiFi e MQTT e reconecta tenta reconectar caso não tenha
   Parâmetros: nenhum
   Retorno: nenhum
*/
void verificaConexoesWifiAndMQTT() {
  // Se não há conexão com o WiFI, a conexão é refeita, se existe, nada é feito
  reconectWiFi();
  //Se não há conexão com o Broker, tenta refazer a conexão
  reconnectMQTT();
}

/*
   Função: Reconecta-se ao WiFi caso não esteja conectado.
   Parâmetros: nenhum
   Retorno: nenhum
*/
void reconectWiFi() {

  // Se já está conectado a rede WiFi, nada é feito.
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  // Caso contrário, conecta na rede WiFi
  else {
    while(WiFi.status() == WL_IDLE_STATUS){
      
      WiFi.begin(SSID, PASSWORD);
      
      uint32_t start_time = millis();
      while((millis() - start_time) < 1200){} // Espera 1 segundo antes de tentar novamente se conectar ao wifi
                                              // Utilizando a millis para ter uma melhor precisão
    }

    timeClient.begin(); // sincroniza relogio
    timeClient.update(); // atualiza relogio
  }

}

/*
   Função: Reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
   Parâmetros: nenhum
   Retorno: nenhum
*/
void reconnectMQTT() {
  if (!MQTT.connected()) {
    if (MQTT.connect(ID_MQTT)) {
      return;
    }
    else {
      // Não conseguiu conectado
    }
  }
  else {
    // Está conectado
    return;
  }
}

/*
   Função: Envia ao Broker a mensagem corrente + data em formato ISO
   Parâmetros: nenhum
   Retorno: nenhum
*/
void enviaEstadoOutputMQTT(void) {
  if (!MQTT.connected()) {
    //TODO: Coloca num buffer
  }
  else {
    // Envia ao Broker o estado atual do output
    MQTT.publish(TOPICO_PUBLISH, mensagem.c_str());
  }

}

/*
   Função: Ler dados do sensor e realizar calculo da media da corrente através dos picos da corrente no intervalo de 1 segundo
   Parâmetros: nenhum
   Retorno: Valor médio do pico de correntes em um intervalo de 1 segundo
*/
float getSensor() {
  
  /* Inicia variáveis locais */
  int iter = 0;
  int leituraValor;
  float valorSensor = 0;
  float valorCorrente = 0;
  float voltsporUnidade = 0.00322581;// 3.3%1023

  float sensibilidade = 0.185;// Para ACS712 de  5 Amperes use 0.185
                              // Para ACS712 de 10 Amperes use 0.100
                              // Para ACS712 de 30 Amperes use 0.066

  int valorMin = 1024;
  int valorMax = 0;
  uint32_t start_time = millis();
  //objetivo desse loop é fazer a leituras dos picos em um intervalo de 1 segundo e realizar o calculo baseado no menor e no maior pico
  while ((millis() - start_time) < 1200) // Amostra de 1 segundo
  {
    // Lê o status atual do sensor
    leituraValor  = (analogRead(pinoSensor_ACS712_5A));

    if (leituraValor > valorMax) {

      valorMax = leituraValor;

    }

    if ( leituraValor < valorMin) {

      valorMin = leituraValor;

    }

  }

  valorSensor = (((valorMax - valorMin) * 3.4)) / 1024;

  valorCorrente = ((valorSensor / 2) * 0.707) / sensibilidade ;

  return valorCorrente;
  
}
