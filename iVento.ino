#include "WiFiEsp.h" //INCLUSÃO DA BIBLIOTECA
#include "SoftwareSerial.h"//INCLUSÃO DA BIBLIOTECA
#include <LiquidCrystal.h> // Load a LCD library
#include <DHT.h> // Load a DHT biblioteca 

//Constants
#define DHTPIN 2 // what pin we're connected to
#define DHTTYPE DHT22 // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino

//Define os pinos que serão ligados ao LCD
LiquidCrystal lcd(4, 5, 8, 9, 10, 11);

//Array simbolo grau
byte grau[8] = { B00001100,
                 B00010010,
                 B00010010,
                 B00001100,
                 B00000000,
                 B00000000,
                 B00000000,
                 B00000000,
               };

SoftwareSerial Serial1(6, 7); //PINOS QUE EMULAM A SERIAL, ONDE O PINO 6 É O RX E O PINO 7 É O TX

char ssid[] = "ENTER_WIFI_NAME_WHERE"; //VARIÁVEL QUE ARMAZENA O NOME DA REDE SEM FIO
char pass[] = "ENTER_PASSWORD_HERE";//VARIÁVEL QUE ARMAZENA A SENHA DA REDE SEM FIO

int status = WL_IDLE_STATUS; //STATUS TEMPORÁRIO ATRIBUÍDO QUANDO O WIFI É INICIALIZADO E PERMANECE ATIVO
//ATÉ QUE O NÚMERO DE TENTATIVAS EXPIRE (RESULTANDO EM WL_NO_SHIELD) OU QUE UMA CONEXÃO SEJA ESTABELECIDA
//(RESULTANDO EM WL_CONNECTED)

WiFiEspServer server(80); //CONEXÃO REALIZADA NA PORTA 80

RingBuffer buf(8); //BUFFER PARA AUMENTAR A VELOCIDADE E REDUZIR A ALOCAÇÃO DE MEMÓRIA

//Variables
double count = 600001;
float hum;  //Stores humidity value
float temp; //Stores temperature value

int statusLed = LOW; //VARIÁVEL QUE ARMAZENA O ESTADO ATUAL DO LED (LIGADO / DESLIGADO)

void setup(){
  dht.begin(); // Inicia o sensor DHT22
  lcd.begin(16, 2); //Inicializa LCD
  lcd.clear(); //Limpa o LCD
  //Cria o caractere customizado com o simbolo do grau
  lcd.createChar(0, grau);
  pinMode(3, OUTPUT); //DEFINE O PINO COMO SAÍDA (PINO 3 --> Relê)
  digitalWrite(3, LOW); //PINO 3 INICIA DESLIGADO
  Serial.begin(9600); //INICIALIZA A SERIAL
  Serial1.begin(9600); //INICIALIZA A SERIAL PARA O ESP8266
  WiFi.init(&Serial1); //INICIALIZA A COMUNICAÇÃO SERIAL COM O ESP8266
  WiFi.config(IPAddress(192,168,15,110)); //COLOQUE UMA FAIXA DE IP DISPONÍVEL DO SEU ROTEADOR

  //INÍCIO - VERIFICA SE O ESP8266 ESTÁ CONECTADO AO ARDUINO, CONECTA A REDE SEM FIO E INICIA O WEBSERVER
  if(WiFi.status() == WL_NO_SHIELD){
    while (true);
  }
  while(status != WL_CONNECTED){
    status = WiFi.begin(ssid, pass);
  }
  server.begin();
  //FIM - VERIFICA SE O ESP8266 ESTÁ CONECTADO AO ARDUINO, CONECTA A REDE SEM FIO E INICIA O WEBSERVER
}

void loop(){

  if (count > 600000) {
     readHumidityTemp();
  }
  
  count++;
  
  WiFiEspClient client = server.available(); //ATENDE AS SOLICITAÇÕES DO CLIENTE

  if (client) { //SE CLIENTE TENTAR SE CONECTAR, FAZ
    buf.init(); //INICIALIZA O BUFFER
    while (client.connected()){ //ENQUANTO O CLIENTE ESTIVER CONECTADO, FAZ
      if(client.available()){ //SE EXISTIR REQUISIÇÃO DO CLIENTE, FAZ
        char c = client.read(); //LÊ A REQUISIÇÃO DO CLIENTE
        buf.push(c); //BUFFER ARMAZENA A REQUISIÇÃO

        //IDENTIFICA O FIM DA REQUISIÇÃO HTTP E ENVIA UMA RESPOSTA
        if(buf.endsWith("\r\n\r\n")) {
          sendHttpResponse(client);
          break;
        }
        if(buf.endsWith("GET /H")){ //SE O PARÂMETRO DA REQUISIÇÃO VINDO POR GET FOR IGUAL A "H", FAZ 
            digitalWrite(3, HIGH); //ACENDE O LED
            statusLed = 1; //VARIÁVEL RECEBE VALOR 1(SIGNIFICA QUE O LED ESTÁ ACESO)
        }
        else{ //SENÃO, FAZ
          if (buf.endsWith("GET /L")) { //SE O PARÂMETRO DA REQUISIÇÃO VINDO POR GET FOR IGUAL A "L", FAZ
                  digitalWrite(3, LOW); //APAGA O LED
                  statusLed = 0; //VARIÁVEL RECEBE VALOR 0(SIGNIFICA QUE O LED ESTÁ APAGADO)
          }
        }
      }
    }
    client.stop(); //FINALIZA A REQUISIÇÃO HTTP E DESCONECTA O CLIENTE
  }
}

void readHumidityTemp()
{

  count = 0;
  
  //Intervalo recomendado para leitura do sensor de 2 segundos
  delay(2000);

  //Read data and store it to variables hum and temp
  hum = dht.readHumidity();
  temp = dht.readTemperature();

  //Print temp and humidity values to serial monitor
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");

  //Exibe os dados no display
  lcd.setCursor(0, 0);
  lcd.print("Temp : ");
  lcd.print(" ");
  lcd.setCursor(7, 0);
  lcd.print(temp, 1);
  lcd.setCursor(12, 0);

  //Mostra o simbolo do grau formado pelo array
  lcd.write((byte)0);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Umid : ");
  lcd.print(" ");
  lcd.setCursor(7, 1);
  lcd.print(hum, 1);
  lcd.setCursor(12, 1);
  lcd.print("%");

}

//MÉTODO DE RESPOSTA A REQUISIÇÃO HTTP DO CLIENTE
void sendHttpResponse(WiFiEspClient client){
  client.println("HTTP/1.1 200 OK"); //ESCREVE PARA O CLIENTE A VERSÃO DO HTTP
  client.println("Content-Type: text/html"); //ESCREVE PARA O CLIENTE O TIPO DE CONTEÚDO(texto/html)
  client.println("");
  client.println("<!DOCTYPE HTML>"); //INFORMA AO NAVEGADOR A ESPECIFICAÇÃO DO HTML
  client.println("<html>"); //ABRE A TAG "html"
  client.println("<head>"); //ABRE A TAG "head"
  https://www.fiap.com.br/wp-content/themes/fiap2016/images/sharing/fiap.png
  //client.println("<link rel='icon' type='image/png' href='https://blogmasterwalkershop.com.br/arquivos/artigos/sub_wifi/icon_mws.png'/>"); //DEFINIÇÃO DO ICONE DA PÁGINA
  client.println("<link rel='stylesheet' type='text/css' href='https://blogmasterwalkershop.com.br/arquivos/artigos/sub_wifi/webpagecss.css' />"); //REFERENCIA AO ARQUIVO CSS (FOLHAS DE ESTILO)
  client.println("<title>iVento - Dorme com essa!</title>"); //ESCREVE O TEXTO NA PÁGINA
  client.println("</head>"); //FECHA A TAG "head"
  
  //AS LINHAS ABAIXO CRIAM A PÁGINA HTML
  client.println("<body>"); //ABRE A TAG "body"
  //client.println("<img alt='masterwalkershop' src='https://blogmasterwalkershop.com.br/arquivos/artigos/sub_wifi/logo_mws.png' height='156' width='700' />"); //LOGO DA MASTERWALKER SHOP
  client.println("<p style='line-height:2'><font>iVento - Dorme com essa!</font></p>"); //ESCREVE O TEXTO NA PÁGINA
  
  client.println("<p style='line-height:0'><font color='green'>Temperatura: ");
  client.println(temp);
  client.println("&#8451 </font></p>"); //ESCREVE "LIGADO" NA PÁGINA
  client.println("<p style='line-height:0'><font color='green'>umidade: "); 
  client.println(hum);
  client.println("% </font></p>"); //ESCREVE "LIGADO" NA PÁGINA
  
  client.println("<font>ESTADO ATUAL DO LED</font>"); //ESCREVE O TEXTO NA PÁGINA
  
  if (statusLed == HIGH){ //SE VARIÁVEL FOR IGUAL A HIGH (1), FAZ
    client.println("<p style='line-height:0'><font color='green'>LIGADO</font></p>"); //ESCREVE "LIGADO" NA PÁGINA
    client.println("<a href=\"/L\">APAGAR</a>"); //COMANDO PARA APAGAR O LED (PASSA O PARÂMETRO /L)
  }else{ //SENÃO, FAZ
    if (statusLed == LOW){ //SE VARIÁVEL FOR IGUAL A LOW (0), FAZ
    client.println("<p style='line-height:0'><font color='red'>DESLIGADO</font></p>"); //ESCREVE "DESLIGADO" NA PÁGINA
    client.println("<a href=\"/H\">ACENDER</a>"); //COMANDO PARA ACENDER O LED (PASSA O PARÂMETRO /H)
    }
  }
  client.println("<hr />"); //TAG HTML QUE CRIA UMA LINHA NA PÁGINA
  client.println("<hr />"); //TAG HTML QUE CRIA UMA LINHA NA PÁGINA
  client.println("</body>"); //FECHA A TAG "body"
  client.println("</html>"); //FECHA A TAG "html"
  delay(1); //INTERVALO DE 1 MILISSEGUNDO
}
