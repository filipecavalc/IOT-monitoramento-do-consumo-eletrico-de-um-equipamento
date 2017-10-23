Antes de executar o código certifique-se de que você possui tudo oque é necessario.
Como configurar a IDE Arduino para o Wemos e outros micro-controladores derivados do ESP8266: https://goo.gl/q3SYvy


## Bibliotecas <br />
Links para download das bibliotecas extras necessarias: <br />
PubSubClient.h - https://goo.gl/xsstvb <br />
NTPClient.h    - https://goo.gl/Fe1jGF <br />
TimeLib.h      - https://goo.gl/TGHBP4 <br />
```c
#include <ESP8266WiFi.h>  // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient do MQTT
#include <NTPClient.h>    // Importa a Biblioteca NTPClient para conexão com servidor NTP
                          // para coletar o horario em formato Epoch
#include <WiFiUdp.h>      // Importa a Biblioteca WiFiUdp para permitir enviar e receber pacotes via UDP
#include <Time.h>         // Complemento da Biblioteca TimeLib.h
#include <TimeLib.h>      // Importa Biblioteca para realizar a conversão do formato Epoch
                          // para ISO no metodo epochToLabIoTFormat
```
# Configuração do protocolo MQTT <br />
```c
#define TOPICO_PUBLISH "consumo_Eletrico_LabIoT" // tópico aonde será enviado a mensagem
#define ID_MQTT  "id_consumo_Eletrico_LabIoT_001" // nome do dispositivo
const char* BROKER_MQTT = ""; //URL ou IP do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT normalmente a 1883
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

/*
   Função: Inicializa MQTT e informa qual broker e porta deve ser conectado
   Parâmetros: nenhum
   Retorno: nenhum
*/
void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
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
```
# Configuração para conexão na rede <br />
```c
const char* SSID = ""; // SSID nome da rede WI-FI que deseja se conectar
const char* PASSWORD = ""; // Senha da rede WI-FI que deseja se conectar

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
```
# Configuração para coleta do horario via internet <br />
```c
WiFiUDP ntpUDP;//biblioteca para envio de mensagem via protocolo UDP
int16_t utc = -3; //UTC -3:00 Brazil
NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utc * 3600, 60000);// NTP get Time 
                                                                // Configurações para consulta
                                                                // do horario atual via internet
```
# Leitura do sensor e Cálculo da corrente
```c
// Constantes Globais para calculo do consumo
float mediaCorrente = 0;
int tensaoRedeEletrica = 127; // 127 ou ou 220 dependendo da tensão da tomada aonde for ser realizado a medição

// Portas dos entrada do Sensor
#define pinoSensor_ACS712_5A A0

// Constantes Globais utilizadas no loop
String mensagem = "";
unsigned int horario = 0;

/*
   Função: Ler dados do sensor e realizar calculo da media da corrente
           através dos picos da corrente no intervalo de 1 segundo
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
  //objetivo desse loop é fazer a leituras dos picos em um intervalo de 1 segundo e realizar o calculo
  //baseado no menor e no maior pico
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
```
# Código do Setup
```c
void setup() {
  //Incia a Serial
  Serial.begin(9600);
  pinMode(pinoSensor_ACS712_5A, INPUT);
  reconectWiFi(); // conecta-se a rede WIFI
  initMQTT(); // conecta-se ao broker MQTT
}
```
# Código do Loop
```c
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
}
```
# Conversor de formato epoch para ISO
```c
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
```

|Materiais e Componentes| 
| ------ |
| WEMOS D1 MINI PRO Mais informações neste link: [GUIA WEMOS D1 MINI PRO](https://goo.gl/Gs3dgQ) <br /> <img alt="WEMOS D1 MINI PRO SCHEMATIC" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Materiais%20e%20componentes/wemos_d1_mini_pro_pinout.png" width="250"> |
| ANTENA WEMOS D1 MINI PRO. Sem detalhes, antena padrão e conector padrão, pode ser obtida no kit do WEMOS. <br /> <img alt="ANTENA WEMOS D1 MINI PRO" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Materiais%20e%20componentes/antena.png" width="150"> |
| ACS712 5A. (Vale o aviso de que o output do sensor foi ligado diretamente no analogico do WEMOS D1 MINI PRO, pois o mesmo trabalha em uma faixa de até 3.4 volts no output, outro modelo de ACS712 trabalham com tensões mais altas no output, então o circuito deverá ser revisitado com as devidas alterações) Mais informações neste link: [Manual do usuario sensor de corrente ACS712 5A](https://goo.gl/tzyDiZ) <br /> <img alt="ACS712 SCHEMATIC" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Materiais%20e%20componentes/ACS-712-Pinouts.png" width="250"> |
| FONTE YS-12V450A usada para alimentação em 127V ou 220V. Possui saída 5V permitindo ligar o WEMOS D1 MINI PRO e o Sensor ACS712 5A. Mais informações neste link: [YS-12V450A Esquemático](https://goo.gl/hyTrNA) <br /> <img alt="YS-12V450A SCHEMATIC" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Materiais%20e%20componentes/ys-12v450a-Schematic.png" width="260"><img alt="YS-12V450A" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Materiais%20e%20componentes/ys-12v450a.png" width="150"> |
| 2 x BORNE. Mais informações neste link: [Conector Borne KRE 2 Vias](https://goo.gl/vKmvR2) <br /> <img alt="Borne" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Materiais%20e%20componentes/Borne.png" width="150"> |
| Conector macho <br /> <img alt="Conector Macho" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Materiais%20e%20componentes/conector_macho.png" width="150"> |
| Modulo Tomada de energia <br />  <img alt="Modulo Tomada de energia" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Materiais%20e%20componentes/tomada1.png" width="150"> <img alt="Modulo Tomada de energia" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Materiais%20e%20componentes/tomada2.png" width="150"> <img alt="Modulo Tomada de energia" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Materiais%20e%20componentes/tomada3.png" width="150">|
| Placa de Fenolite Perfurada <br /> <img alt="Placa de Fenolite Perfurada" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Materiais%20e%20componentes/placa-fenolite-perfurada.png" width="150"> |


| Protótipagem | 
| ------ |
| Circuito Elétrico. Segue o link para download do arquivo editavel: [Circuito Elétrico easyeda](https://goo.gl/VcahGX) <br /> <img alt="Circuito Monitor Consumo Eletrico IOT" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Projeto%20Circuito%20e%20PCB/IOTEnergia_circuit.png" width="400"> |
| Protótipo PCB Segue o link para download do arquivo editavel: [Projeto PCB easyeda](https://goo.gl/xMyhBx) <br /> <img alt="PCB Monitor Consumo Eletrico IOT" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Projeto%20Circuito%20e%20PCB/IOTEnergia_PCB.png" width="400"> |
| Protótipo implementado em uma Placa de Fenolite Perfurada <br /> <img alt="Protótipo Placa Fenolite Perfurada Frente" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Prot%C3%B3tipo%20fenolite%20perfurado/Prototipo_circuito_fenolite_perfurado_cima.png" width="150"> <img alt="Protótipo Placa Fenolite Perfurada Trás" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Prot%C3%B3tipo%20fenolite%20perfurado/Prototipo_circuito_fenolite_perfurado_baixo.png" width="150">|
|Versão teste de case para a placa de fenolite e demais componentes <br /> <img alt="Case Tampa" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Case%20pr%C3%B3totipo/Tampa_case.png" width="150"> <img alt="Case Entrada Energia" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Case%20pr%C3%B3totipo/Entrada_Energia_case.png" width="150"> <img alt="Case Saida Energia" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Case%20pr%C3%B3totipo/Saida_Energia_case.png" width="150"> <img alt="Case Serial Wemos" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Case%20pr%C3%B3totipo/Wemos_usb.png" width="150"> <img alt="Antena Case" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Case%20pr%C3%B3totipo/Antena_case.png" width="150">|
| Versão Final do Case, Arquivos em extensão solid nesse link: [Pasta no github com os arquivos em extensão para edição no solid](https://goo.gl/ZwWYU4) e extensão STL para impressão 3D neste link: [Pasta no github com os arquivos STL](https://goo.gl/EsVQsW) <br /> <img alt="case final peças" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Case%20Vers%C3%A3o%20Final/Case_Final.PNG" width="200"> <img alt="case final montagem" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Case%20Vers%C3%A3o%20Final/Case_Final_Paredes.PNG" width="150"> <img alt="case final montagem" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Case%20Vers%C3%A3o%20Final/Case_Final_completo2.PNG" width="150"> <img alt="case final montagem" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/Case%20Vers%C3%A3o%20Final/Case_final_completo.PNG" width="150"> |
