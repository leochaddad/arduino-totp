# Gerador de Time based One-Time-Password em Hardware

## Integrantes do grupo
- Leonardo Cury Haddad - 18.00442-3
- João Pedro de Padua Santoro Azevedo - 18.02277-4
- Gabriel de Godoy Braz - 17.00163-3
- Matheus Lopes Vivas - 17.04401-4

## Componentes
- Arduino Nano
- Módulo RTC DS1302
- Display OLED 128x32 SPI SSD1306

## Descrição do projeto
Nosso projeto gera um código numérico de 6 digitos para autenticação de duas etapas em serviços que suportam o padrão OAUTH (O mesmo utilizado por apps como Authy e Google Authenticator).

O módulo RTC é utilizado para obter o horário atual de maneira precisa, enquanto o token é exibido no display OLED (comunicação por SPI).

## Descrição do Firmware
### Exibição na tela
Foi utilizada bibliotecas da Adafruit para o display, além da biblioteca de SPI e Wire.
O setup é executado na função oledSetup, e a função  drawScreen faz a exibição do Token.
### Tempo
É necessário configurar a data e hora atual do módulo RTC que será utilizada para a geração do token. 
Ao enviar o comando 0, o usuário entra no modo de configuração e deve enviar o tempo no padrão Unix.
A bliblioteca TimeLib faz a conversão entre Unix e human-readable já que a biblioteca do RTC que usamos não fornece o tempo no formato Unix. 

### Geração do Token
Para gerarmos um one-time-password precisamos do tempo Unix atual, período de atualização, número de digitos e algorítmo de hash. 
Utilizamos SHA-1 com atualização em 30s e 6 dígitos que é a configuração mais comum.
A chave privada deve ser convertida de base32 para um byte array, assim como o tempo Unix (big-endian). 
Calcula-se o HMAC-SHA1(k, T0/Tx), onde k é a chave privada, T0 o tempo Unix e Tx o intervalo,  truncamos o valor para o número desejado de caracteres e adicionamos um padding de zeros.
