
### 1. Projeto
**Desenvolvimento de uma Solução IoT para Monitoramento de Temperatura Utilizando a Placa BitDogLab**

---

#### 2. Descrição
Este projeto visa desenvolver uma solução IoT para monitoramento de temperatura utilizando a placa BitDogLab. A solução integra sensores, atuadores e comunicação de rede para coletar dados de temperatura, exibi-los em um display OLED e enviá-los para a nuvem utilizando o serviço ThingSpeak. O sistema é projetado para ser utilizado em aplicações de automação residencial, proporcionando maior controle e eficiência energética.

---

#### 3. Tecnologias Utilizadas
- **Placa BitDogLab**: Microcontrolador utilizado para integrar todos os componentes do projeto.
- **Componentes Específicos**:
  - Botões
  - Display OLED
  - LEDs RGB
  - Sensor de Temperatura
- **Tecnologias de Comunicação**:
  - Wi-Fi
  - Protocolo HTTP
- **Linguagem de Programação**: C

---

#### 4. Como Instalar e Configurar o Projeto
1. **Clone o Repositório**:
   ```bash
   git clone https://github.com/seu-usuario/seu-repositorio.git
   cd seu-repositorio
   ```

2. **Instale as Dependências**:
   - Certifique-se de ter o SDK do Raspberry Pi Pico instalado.
   - Instale as bibliotecas necessárias para o display OLED e Wi-Fi.

3. **Configure o Projeto**:
   - Edite o arquivo `parametros.h` para configurar as credenciais Wi-Fi e outros parâmetros necessários:
     ```c
     #define WIFI_SSID "seu-ssid"
     #define WIFI_PASS "sua-senha"
     #define THINGSPEAK_HOST "api.thingspeak.com"
     #define THINGSPEAK_PORT 80
     #define API_KEY "sua-chave-api"
     ```

4. **Compile e Carregue o Código**:
   - Utilize o ambiente de desenvolvimento de sua preferência para compilar e carregar o código na placa BitDogLab.

---

#### 5. Como Usar o Projeto
1. **Inicialize o Sistema**:
   - Conecte a placa BitDogLab à fonte de alimentação.
   - O sistema irá inicializar e tentar conectar ao Wi-Fi.

2. **Monitoramento de Temperatura**:
   - A temperatura será lida periodicamente e exibida no display OLED.
   - Os LEDs RGB indicarão a faixa de temperatura.

3. **Envio de Dados para a Nuvem**:
   - Os dados de temperatura serão enviados automaticamente para o ThingSpeak a cada 15 segundos.

   - Gráfico dos dados:  https://thingspeak.mathworks.com/channels/2841648

4. **Interação Manual**:
   - Pressione o botão para ler a temperatura manualmente e enviar os dados para o ThingSpeak.

---

#### 6. Informações sobre os Autores
- **Nome**: Diego Gomes
- **GitHub**: limad1

---

#### 7. Qualquer Outra Informação Relevante
- **Licença**: Este projeto está licenciado sob a Licença MIT.
- **Contribuições**: Contribuições são bem-vindas! Sinta-se à vontade para abrir issues e pull requests.
- **Agradecimentos**: Agradecemos aos tutores e professores do curso de Capacitação em Sistemas Embarcados pela orientação e suporte.

---

### Explicação das Funções do Código

#### Função `init_oled`
Inicializa o display OLED configurando a comunicação I2C e limpando a tela.

#### Função `calculate_position`
Calcula a posição X e Y para centralizar o texto no display OLED.

#### Função `display_message`
Exibe uma mensagem no display OLED, centralizando o texto.

#### Função `read_temperature`
Lê a temperatura do sensor interno (ADC4), converte o valor bruto para tensão e depois para temperatura em Celsius.

#### Função `update_leds`
Atualiza os LEDs RGB com base na temperatura lida.

#### Função `connect_to_wifi`
Tenta conectar ao Wi-Fi, exibindo uma mensagem de "loading" e atualizando a cada tentativa.

#### Função `http_recv_callback`
Callback para quando uma resposta é recebida do ThingSpeak, exibindo uma mensagem e liberando o buffer.

#### Função `http_connected_callback`
Callback para quando a conexão TCP é estabelecida, lê a temperatura e envia uma requisição HTTP para o ThingSpeak.

#### Função `dns_callback`
Callback para quando a resolução de DNS é concluída, tenta conectar ao servidor ThingSpeak.

#### Função `repeating_timer_callback`
Callback do temporizador, define a flag `timer_fired` como verdadeira.

#### Função `main`
Inicializa os componentes, tenta conectar ao Wi-Fi, configura o temporizador e entra em um loop infinito para verificar o botão e o temporizador, lendo a temperatura e atualizando os LEDs e o display OLED.

