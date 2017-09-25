# IOT Monitoramento do consumo eletrico de um equipamento
Estudo e desenvolvimento de um dispositivo IoT para o monitoramento do consumo de uma tomada coletando, armazenando e disponibilizando dados coletados do dispositivo IoT na WEB.


Equipamentos:

WEMOS D1 MINI PRO. Mais informações neste link: https://wiki.wemos.cc/products:d1:d1_mini_pro
<img alt="WEMOS D1 MINI PRO SCHEMATIC" src="https://github.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/blob/master/wemos_d1_mini_pro_pinout.png" width="800">

ANTENA WEMOS D1 MINI PRO. Sem detalhes, antena padrão e conector padrão, pode ser obtida no kit do WEMOS.
<img alt="ANTENA WEMOS D1 MINI PRO" src="https://github.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/blob/master/antena.jpg" width="250">

ACS712 5A. (Vale o aviso de que o output do sensor foi ligado diretamente no analogico do WEMOS D1 MINI PRO, pois o mesmo trabalha em uma faixa de até 3.4 volts no output, outro modelo de ACS712 trabalham com tensões mais altas no output, então o circuito deverá ser revisitado com as devidas alterações) Mais informações neste link: http://henrysbench.capnfatz.com/henrys-bench/arduino-current-measurements/acs712-current-sensor-user-manual/
<img alt="ACS712 SCHEMATIC" src="https://raw.githubusercontent.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/master/ACS-712-Pinouts.png" width="500">

FONTE YS-12V450A usada para alimentação em 127V ou 220V. Possui saída 5V permitindo ligar o WEMOS D1 MINI PRO e o Sensor ACS712 5A. Main informações neste link: http://shop.cpu.com.tw/html/2587/AD-0507.html

| <img alt="YS-12V450A SCHEMATIC" src="https://github.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/blob/master/ys-12v450a-Schematic.jpg" width="500"> | <img alt="YS-12V450A" src="https://github.com/filipecavalc/IOT-monitoramento-do-consumo-eletrico-de-um-equipamento/blob/master/ys-12v450a.JPG" width="500"> |
| ------ | ------ |
