#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <time.h>
#include <unistd.h>
#include "esp_timer.h"


#include "driver/touch_pad.h"
#include "soc/rtc_periph.h"
#include "soc/sens_periph.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "freertos/semphr.h"

static const char *TAG = "Touch pad";

#define TOUCH_THRESH_NO_USE (0)
#define TOUCH_THRESH_PERCENT (80)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

static bool s_pad_activated[TOUCH_PAD_MAX];
static uint32_t s_pad_init_val[TOUCH_PAD_MAX];

//Declaração de variaveis
int temperatura = 0;
int airbag = 0;
int ativarAbs = 0;
int vidros = 0;
int luzes = 0;
int travas = 0;
int cintoPassageiro = 0;
int cintoMotorista = 0;
int injecaoEletronica = 0;
int velocidade = 0;


int ativandoVidros = 0;   // Touch 3
int temp = 0;             // Touch 4
int ativandoA = 0;        // Touch 5
int ativandoAbs = 0;      // Touch 6
int ativandoCM = 0;       // Touch 8
int ativandoCP = 0;       // Touch 7
int ativandoInjecaoE = 0; // Touch 9

//Declarando variaveus de tempo
int64_t fimVdr, inicioVdr;
uint64_t fimTem, inicioTem;
uint64_t fimAbs, inicioAbs;
uint64_t fimAir, inicioAir;
uint64_t fimCP, inicioCP;
uint64_t fimCM, inicioCM;
uint64_t fimIE, inicioIE;

//Declarando variaveis para calcular o tempo de execução
double tempoVdr =0;
double tempoTem = 0;
double tempoAbs = 0;
double tempoAir = 0;
double tempoCP = 0;
double tempoCM = 0;
double tempoIE = 0;


//inicializando variavel para o mutex
SemaphoreHandle_t mutual_exclusion_mutex;

//Display para mostrar os resultados
void display()
{
    while (1)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        system("cls");
        xSemaphoreTake(mutual_exclusion_mutex, portMAX_DELAY);
        printf("\t\t------\tMONITORAMENT DO VEICULO\t-----\n\n");

        if (vidros == 0)
        {
            printf("\nVidros dianteiros                   |\t Desativada");
        }
        else
        {
            printf("\nVidros dianteiros                   |\t Ativada \t          |  Tempo de execução: %lf", tempoIE);
        }

        if (injecaoEletronica == 0)
        {
            printf("\nInjeção eletronica                  |\t Desativada");
        }
        else
        {
            printf("\nInjeção eletronica                  |\t Ativada \t          |  Tempo de execução: %lf", tempoIE);
        }
        printf("\n--------------------------------------------------------------------------------");
        if (temperatura == 1)
        {
            printf("\nTemperatura motor                   |\t Alta \t                             |  Tempo de execução: %lf", tempoTem);
            printf("\n--------------------------------------------------------------------------------");
        }
        else
        {
            printf("\nTemperatura motor                   |\t Normal");
            printf("\n--------------------------------------------------------------------------------");
        }
        if (ativarAbs == 0)
        {
            printf("\nAbs Dianteiro                       |\t Desativado");
        }
        else
        {
            printf("\nAbs Dianteiro                       |\t Ativado  \t                        |  Tempo de execução: %lf", tempoAbs);
        }
        printf("\n--------------------------------------------------------------------------------");
        if (airbag == 0)
        {
            printf("\nAirbag                              |\t Desativado");
        }
        else
        {
            printf("\nAirbag                              |\t Ativado  \t                        | Tempo de execução: %lf", tempoAir);
        }
        printf("\n--------------------------------------------------------------------------------");
        if (cintoPassageiro == 0)
        {
            printf("\nCinto de segurança do passageiro    |\t Sem Cinto de Segurança");
        }
        else
        {
            printf("\nCinto de segurança do passageiro    |\t Com Cinto de Segurança \t          |  Tempo de execução: %lf", tempoCP);
        }
        printf("\n--------------------------------------------------------------------------------");
        if (cintoMotorista == 0)
        {
            printf("\nCinto de segurança do motorista     |\t Sem Cinto de Segurança");
        }
        else
        {
            printf("\nCinto de segurança do motorista     |\t Com Cinto de Segurança  \t         |  Tempo de execução: %lf", tempoCM);
        }
        printf("\n--------------------------------------------------------------------------------\n\n\n");
        xSemaphoreGive(mutual_exclusion_mutex);
    }
}

static void sensorVidros(void)
{
    while (1)
    {
        
        if (vidros == 1)
        {
            vTaskDelay(20 / portTICK_PERIOD_MS);
            xSemaphoreTake(mutual_exclusion_mutex, portMAX_DELAY);
            if(vidros == 0){
                vidros = 1;
            } else {
                vidros = 0;
            }
            ativandoVidros = 0;
            xSemaphoreGive(mutual_exclusion_mutex);
            fimVdr = esp_timer_get_time();
            tempoVdr = ((fimVdr - inicioVdr)/1000);

        }
        vTaskDelay(10 / portTICK_PERIOD_MS);

    }
}

static void sensorVidros(void)
{
    while (1)
    {
        
        if (vidros == 1)
        {
            vTaskDelay(20 / portTICK_PERIOD_MS);
            xSemaphoreTake(mutual_exclusion_mutex, portMAX_DELAY);
            if(vidros == 0){
                vidros = 1;
            } else {
                vidros = 0;
            }
            ativandoVidros = 0;
            xSemaphoreGive(mutual_exclusion_mutex);
            fimVdr = esp_timer_get_time();
            tempoVdr = ((fimVdr - inicioVdr)/1000);

        }
        vTaskDelay(10 / portTICK_PERIOD_MS);

    }
}
static void set_thresholds(void)
{
    uint16_t touch_value;
    for (int i = 0; i < TOUCH_PAD_MAX; i++)
    {
        // Ler valor filtrado
        touch_pad_read_filtered(i, &touch_value);
        s_pad_init_val[i] = touch_value;
        // Define o limite da interrupção.
        ESP_ERROR_CHECK(touch_pad_set_thresh(i, touch_value * 2 / 3));
    }
}

static void tab_touch_pad_init(void)
{
    for (int i = 0; i < TOUCH_PAD_MAX; i++)
    {
        // iniciar RTC IO e modo para touch pad.
        touch_pad_config(i, TOUCH_THRESH_NO_USE);
    }
}

//Função para verificar se houve interrupção
static void interrupcao(void *arg)
{
    uint32_t pad_intr = touch_pad_get_status();
    // clear interrupt
    touch_pad_clear_status();
    for (int i = 0; i < TOUCH_PAD_MAX; i++)
    {
        if ((pad_intr >> i) & 0x01)
        {
            s_pad_activated[i] = true; //Se ocorreu interrupção atualiza para true
        }
    }
}

//Função para ativar os sensores conforme o touch
static void read_task(void *pvParameter)
{

    while (1)
    {
        touch_pad_intr_enable();
        for (int i = 0; i < TOUCH_PAD_MAX; i++)
        {
            if (s_pad_activated[i] == true) //Verifica qual touch foi ativado
            {
                s_pad_activated[i] = false; //Atualiza para false
                //Realiza a operação conforme qual sensor foi ativado
                
                
                if (i == 3) //Alteração na temperatura
                {
                    inicioTem = esp_timer_get_time();

                    if (temp == 0)
                    {
                        temp = 1;
                    }
                    else
                    {
                        temp = 0;
                    }
                }
                else if (i == 4) //Alteração na temperatura
                {
                    inicioTem = esp_timer_get_time();

                    if (temp == 0)
                    {
                        temp = 1;
                    }
                    else
                    {
                        temp = 0;
                    }
                }
                else if (i == 5) //Alteração no airbag
                {
                    inicioAir = esp_timer_get_time();

                    //se for igual a 0 atualiza para 1, caso contrario para 0
                    if (ativandoA == 0) 
                    {
                        ativandoA = 1;
                    }
                    else
                    {
                        ativandoA = 0;
                    }
                }
                else if (i == 6) //Alteração no Abs
                {
                    inicioAbs = esp_timer_get_time();

                    //se for igual a 0 atualiza para 1, caso contrario para 0
                    if (ativandoAbs == 0)
                    {
                        ativandoAbs = 1;
                    }
                    else
                    {
                        ativandoAbs = 0;
                    }
                }
                else if (i == 7) //Alteração no cinto do passageiro
                {
                    inicioCP = esp_timer_get_time();
                    //se for igual a 0 atualiza para 1, caso contrario para 0
                    if (ativandoCP == 0)
                    {
                        ativandoCP = 1;
                    }
                    else
                    {
                        ativandoCP = 0;
                    }
                }
                else if (i == 8) //Alteração no cinto do motorista
                {
                    inicioCM = esp_timer_get_time();
                    //se for igual a 0 atualiza para 1, caso contrario para 0
                    if (ativandoCM == 0)
                    {
                        ativandoCM = 1;
                    }
                    else
                    {
                        ativandoCM = 0;
                    }
                }
                else if (i == 9)
                {
                    inicioIE = esp_timer_get_time();
                    //se for igual a 0 atualiza para 1, caso contrario para 0
                    if (ativandoInjecaoE == 0)
                    {
                        ativandoInjecaoE = 1;
                    }
                    else
                    {
                        ativandoInjecaoE = 0;
                    }
                }
                i++;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

//SENSOR DA INJEÇÃO ELETRONICA
static void sensorInjecao(void)
{
    while (1)
    {
        
        if (ativandoInjecaoE == 1)
        {
            vTaskDelay(500 / portTICK_PERIOD_MS);
            xSemaphoreTake(mutual_exclusion_mutex, portMAX_DELAY);
            if(injecaoEletronica == 0){
                injecaoEletronica = 1;
            } else {
                injecaoEletronica = 0;
            }
            ativandoInjecaoE = 0;
            xSemaphoreGive(mutual_exclusion_mutex);
            fimIE = esp_timer_get_time();
            tempoIE = ((fimIE - inicioIE)/1000);

        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// SENSOR DA TEMPERATURA INTERNA DO MOTOR
static void sensorTemperatura(void)
{
    while (1)
    {
        
        if (temp == 1)
        {
            vTaskDelay(20 / portTICK_PERIOD_MS);
            xSemaphoreTake(mutual_exclusion_mutex, portMAX_DELAY);
            if(temperatura == 0){
                temperatura = 1;
            } else {
                temperatura = 0;
            }
            temp = 0;
            xSemaphoreGive(mutual_exclusion_mutex);
            fimTem = esp_timer_get_time();
            tempoTem = ((fimTem - inicioTem)/1000);

        }
        vTaskDelay(10 / portTICK_PERIOD_MS);

    }
}

//SENSOR DO AIRBAG
static void sensorAibag(void)
{
    while (1)
    {
        
        if (ativandoA == 1)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            xSemaphoreTake(mutual_exclusion_mutex, portMAX_DELAY);
            if(airbag == 0){
                airbag = 1;
            } else {
                airbag = 0;
            }
            ativandoA = 0;
            xSemaphoreGive(mutual_exclusion_mutex);
            fimAir = esp_timer_get_time();
            tempoAir = ((fimAir - inicioAir)/1000);

        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

//SENSOR DO ABS DIANTEIRO
static void sensorAbs(void)
{
    while (1)
    {
        
        if (ativandoAbs == 1)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            xSemaphoreTake(mutual_exclusion_mutex, portMAX_DELAY);
            if(ativarAbs == 0){
                ativarAbs = 1;
            } else {
                ativarAbs = 0;
            }
            ativandoAbs = 0;
            xSemaphoreGive(mutual_exclusion_mutex);
            fimAbs = esp_timer_get_time();
            tempoAbs = ((fimAbs - inicioAbs)/1000);

        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

//SENSOR DO CINTO DE SEGURANÇA
static void sensorCintoPassageiro(void)
{
    while (1)
    {
        
        if (ativandoCP == 1)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            xSemaphoreTake(mutual_exclusion_mutex, portMAX_DELAY);
            if(cintoPassageiro == 0){
                cintoPassageiro = 1;
            } else {
                cintoPassageiro = 0;
            }
            ativandoCP = 0;
            xSemaphoreGive(mutual_exclusion_mutex);
            fimCP = esp_timer_get_time();
            tempoCP = ((fimCP - inicioCP)/1000);

        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

//SENSOR DO CINTO DE SEGURANÇA
static void sensorCintoMotorista(void)
{
    while (1)
    {
        
        if (ativandoCM == 1)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            xSemaphoreTake(mutual_exclusion_mutex, portMAX_DELAY);
            if(cintoMotorista == 0){
                cintoMotorista = 1;
            } else {
                cintoMotorista = 0;
            }
            ativandoCM = 0;
            xSemaphoreGive(mutual_exclusion_mutex);
            fimCM = esp_timer_get_time();
            tempoCM = ((fimCM - inicioCM)/1000);

        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing touch pad");
    // Inicializando o touch
    touch_pad_init();
    // Definindo o modo FSM do sensor de toque em 'TOUCH_FSM_MODE_TIMER'
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    // Definindo a tensão de referência para carga/descarga
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    // Inicia IO do touch pad
    tab_touch_pad_init();
    // Verifica se tem alguma mudança de capacitancia
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    // Define os limites
    set_thresholds();
    // Realiza o registro das interrupções
    touch_pad_isr_register(interrupcao, NULL);

    mutual_exclusion_mutex = xSemaphoreCreateMutex();

    if (mutual_exclusion_mutex != NULL)
    {
        printf("Mutex was created\n");
    }

    xTaskCreatePinnedToCore(&display, "display", 2048, NULL, 2, NULL, 1); // inicia task para display
    xTaskCreatePinnedToCore(&read_task, "read_task", 2048, NULL, 17, NULL, 1);
    xTaskCreatePinnedToCore(&sensorCintoMotorista, "sensorCintoMotorista", 2048, NULL, 11, NULL, 1);
    xTaskCreatePinnedToCore(&sensorCintoPassageiro, "sensorCintoPassageiro", 2048, NULL, 10, NULL, 1);
    xTaskCreatePinnedToCore(&sensorAbs, "sensorAbs", 2048, NULL, 8, NULL, 0);
    xTaskCreatePinnedToCore(&sensorAibag, "sensorAibag", 2048, NULL, 15, NULL, 0);
    xTaskCreatePinnedToCore(&sensorTemperatura, "sensorTemperatura", 2048, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(&sensorInjecao, "sensorInjecao", 2048, NULL, 14, NULL, 0);
}
