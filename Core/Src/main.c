/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "task.h"
#include "MAX96755.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
__attribute__((used)) uint64_t i = 0x1122334455667788;

// --- 新增：极简通讯指令标志位 ---
volatile uint8_t rx_cmd_ready = 0; 
volatile uint8_t rx_cmd_val = 0;
volatile uint8_t rx_screen_res = 0;
volatile uint8_t rx_lvds_mode = 0;
volatile uint8_t rx_screen_pattern = 0;
volatile uint16_t rx_timing_ha = 0;
volatile uint16_t rx_timing_va = 0;
volatile uint16_t rx_timing_fps_x10 = 0;
volatile uint32_t rx_timing_pclk_hz = 0;
volatile uint16_t rx_timing_hbp = 0;
volatile uint16_t rx_timing_hfp = 0;
volatile uint16_t rx_timing_vbp = 0;
volatile uint16_t rx_timing_vfp = 0;
volatile uint8_t rx_timing_lvds = 0;
volatile uint8_t rx_timing_pattern = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void waitforiotimeout(GPIO_PinState level, uint32_t timeout)
{
  uint32_t start = HAL_GetTick();
  GPIO_PinState state = HAL_GPIO_ReadPin(CRQ_GPIO_Port, CRQ_Pin);
  while(state != level) {
    if(HAL_GetTick() - start > timeout) {
      return;
    }
    state = HAL_GPIO_ReadPin(CRQ_GPIO_Port, CRQ_Pin);
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(1000);
  sys_init();

  EN_BL();
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

  // 1. 默认上电：1920x1080 + 双路 LVDS + 棋盘格
  max96755_apply_screen_preset(0, 0, 1);

  // 2. 强行打开 USART1 的总中断和寄存器接收（极其重要！）
  HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // --- 新增：响应上位机画面切换指令 ---
    if (rx_cmd_ready == 1)
    {
        rx_cmd_ready = 0; 
        if (rx_cmd_val == 0x01) {
            max96755_apply_screen_preset(0, 0, 0);
            HAL_UART_Transmit(&huart1, (uint8_t*)"[MCU] Switch to ColorBar OK (1920/dual)\n", 40, 100);
        }
        else if (rx_cmd_val == 0x02) {
            max96755_apply_screen_preset(0, 0, 1);
            HAL_UART_Transmit(&huart1, (uint8_t*)"[MCU] Switch to CheckBoard OK (1920/dual)\n", 42, 100);
        }
        else if (rx_cmd_val == 0x10) {
            uint8_t r = rx_screen_res & 1;
            uint8_t l = rx_lvds_mode & 1;
            uint8_t p = rx_screen_pattern & 1;
            max96755_apply_screen_preset(r, l, p);
            {
              static const char ack[] = "[MCU] Screen preset applied OK\n";
              HAL_UART_Transmit(&huart1, (uint8_t *)ack, sizeof(ack) - 1U, 100);
            }
        }
        else if (rx_cmd_val == 0x11) {
            max96755_apply_user_timing(
                rx_timing_ha,
                rx_timing_va,
                rx_timing_fps_x10,
                (uint8_t)(rx_timing_lvds & 1U),
                (uint8_t)(rx_timing_pattern & 1U));
            {
              static const char ack[] = "[MCU] Custom timing (MAX96755) OK\n";
              HAL_UART_Transmit(&huart1, (uint8_t *)ack, sizeof(ack) - 1U, 100);
            }
        }
        else if (rx_cmd_val == 0x12) {
            max96755_apply_user_timing_pclk(
                rx_timing_ha,
                rx_timing_va,
                rx_timing_fps_x10,
                rx_timing_pclk_hz,
                (uint8_t)(rx_timing_lvds & 1U),
                (uint8_t)(rx_timing_pattern & 1U));
            {
              static const char ack[] = "[MCU] Custom timing+PCLK (MAX96755) OK\n";
              HAL_UART_Transmit(&huart1, (uint8_t *)ack, sizeof(ack) - 1U, 100);
            }
        }
        else if (rx_cmd_val == 0x13) {
            max96755_apply_user_timing_full(
                rx_timing_ha, rx_timing_va, rx_timing_fps_x10, rx_timing_pclk_hz,
                rx_timing_hbp, rx_timing_hfp, rx_timing_vbp, rx_timing_vfp,
                (uint8_t)(rx_timing_lvds & 1U), (uint8_t)(rx_timing_pattern & 1U));
            {
              static const char ack[] = "[MCU] Custom timing+PCLK+Porch OK\n";
              HAL_UART_Transmit(&huart1, (uint8_t *)ack, sizeof(ack) - 1U, 100);
            }
        }
        // =======================================================
        // --- 终极验货：无视视频时序，直接掐断/开启背光电源！ ---
        else if (rx_cmd_val == 0x03) {
            DISEN_BL(); // 调用你原本的关背光函数
            HAL_UART_Transmit(&huart1, (uint8_t*)"[MCU] Backlight OFF OK\n", 23, 100);
        }
        else if (rx_cmd_val == 0x04) {
            EN_BL();    // 调用你原本的开背光函数
            HAL_UART_Transmit(&huart1, (uint8_t*)"[MCU] Backlight ON OK\n", 22, 100);
        }
        // =======================================================
    }
    
    static uint8_t state_flag = 0;//芯片在线不在线
    static uint32_t c_time = 0;
    if(max96752_check_chip()) {
      if(state_flag == 0) {
        //TODO: config 86752
        HAL_Delay(3000);
        max96752_init_config();
        uint8_t factory[7] = {0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1A}; //第一个是ID, 4个data，1个ext, 1个checksum
        HAL_I2C_Master_Transmit(&hi2c1, 0x34, factory, 7, 100);
        c_time = HAL_GetTick();
        HAL_Delay(50);
        HAL_UART_Transmit(&huart1, "_____NEW_DEVICE_____\n", 63, 100);
      }

	    state_flag = 1;
    }else {
      state_flag = 0;
    }

    if(state_flag == 1) {
      if((HAL_GetTick() - c_time) % 59700 <= 55000){
        //pwm = 1;
        __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 1000);
      }else if((HAL_GetTick() - c_time) % 59700 <= 59500) {
        //pwm = 0.1;
        __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 100);
      }else {
        __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 1000);
      }

	  HAL_Delay(50);
    static uint32_t last_c = 0;
    static uint32_t c =  0;
    static uint8_t inquire_flag = 1;//询问标志位
    c = (HAL_GetTick() - c_time) % 2000;
    if(last_c > c) {
      inquire_flag = 1; //每隔2秒询问一次
    }else {
      inquire_flag = 0; //不询问
    }
    last_c = c;

    if(inquire_flag) {
      //询问命令
      uint8_t inquire[7] = {0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81}; //第一个是ID, 4个data，1个ext, 1个checksum
      HAL_I2C_Master_Transmit(&hi2c1, 0x34, inquire, 7, 100);

      //读取数据
      uint8_t _0x46[11] = {0}; 
      // while(HAL_GPIO_ReadPin(CRQ_GPIO_Port, CRQ_Pin) != GPIO_PIN_RESET);
      waitforiotimeout(GPIO_PIN_RESET, 100);
      HAL_I2C_Master_Receive(&hi2c1, 0x34, _0x46, 11, 100);

      uint8_t _0x48[11] = {0}; 
      waitforiotimeout(GPIO_PIN_RESET, 100);
      HAL_I2C_Master_Receive(&hi2c1, 0x34, _0x48, 11, 100);

      uint8_t _0x4A[11] = {0};
      waitforiotimeout(GPIO_PIN_RESET, 100);
      HAL_I2C_Master_Receive(&hi2c1, 0x34, _0x4A, 11, 100);

      uint8_t _0x4A_E[12] = {0};
      HAL_I2C_Master_Receive(&hi2c1, 0x34, _0x4A_E, 12, 100);

      uint8_t _0x4B[11] = {0};
      waitforiotimeout(GPIO_PIN_RESET, 100);
      HAL_I2C_Master_Receive(&hi2c1, 0x34, _0x4B, 11, 100);

      uint8_t _0x4B_E[12] = {0};
      HAL_I2C_Master_Receive(&hi2c1, 0x34, _0x4B_E, 12, 100);

      uint8_t _0x4C[11] = {0};
      waitforiotimeout(GPIO_PIN_RESET, 100);
      HAL_I2C_Master_Receive(&hi2c1, 0x34, _0x4C, 11, 100);

      uint8_t _0x4C_E[11] = {0};
      HAL_I2C_Master_Receive(&hi2c1, 0x34, _0x4C_E, 11, 100);
	  
      //发送信息
    HAL_UART_Transmit(&huart1, "state:    ", 10, 100);
    if(_0x48[1] == 0) 
      HAL_UART_Transmit(&huart1, "OK!\n", 4, 100);
    else
      HAL_UART_Transmit(&huart1, "FAIL!\n", 6, 100);

    HAL_UART_Transmit(&huart1, "Temperature: ", 13, 100);
    char t[2]	= {_0x48[5]/10 + '0', _0x48[5]%10 + '0'};
    HAL_UART_Transmit(&huart1, t, 2, 100);
    HAL_UART_Transmit(&huart1, "\n", 1, 100);

    HAL_UART_Transmit(&huart1, "software version: ", 18, 100);
    HAL_UART_Transmit(&huart1, &_0x4A[1], 8, 100);
    HAL_UART_Transmit(&huart1, &_0x4A_E[1], 9, 100);
    HAL_UART_Transmit(&huart1, "\n", 1, 100);

    HAL_UART_Transmit(&huart1, "hardware version: ", 18, 100);
    HAL_UART_Transmit(&huart1, &_0x4B[1], 8, 100);
    HAL_UART_Transmit(&huart1, &_0x4B_E[1], 9, 100);
    HAL_UART_Transmit(&huart1, "\n", 1, 100);

      
    }
    HAL_Delay(100);
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_1);
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 72-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, BL_EN_Pin|MAX96755_PDEN_Pin|VDD2_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LCD_RESET_Pin */
  GPIO_InitStruct.Pin = LCD_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BL_EN_Pin */
  GPIO_InitStruct.Pin = BL_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BL_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CRQ_Pin */
  GPIO_InitStruct.Pin = CRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(CRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : MAX96755_PDEN_Pin VDD2_EN_Pin */
  GPIO_InitStruct.Pin = MAX96755_PDEN_Pin|VDD2_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : VDD1_PG_Pin */
  GPIO_InitStruct.Pin = VDD1_PG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(VDD1_PG_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// --- 终极绝杀：纯底层寄存器接收 ---
void Custom_UART_RX_Handler(void)
{
    if ((USART1->SR & (USART_SR_ORE | USART_SR_NE | USART_SR_FE)) != RESET) {
        volatile uint32_t tmp = USART1->SR;
        tmp = USART1->DR;
        (void)tmp;
    }

    if ((USART1->SR & USART_SR_RXNE) != RESET) {
        uint8_t data = USART1->DR; 
        
        // 【硬件回声镜】：收到什么瞬间弹回给电脑！
        while((USART1->SR & (1<<7)) == 0); 
        USART1->DR = data;                 
        
        static uint8_t state = 0;
        static uint8_t cmd = 0;
        static uint8_t timing_blob8[8];
        static uint8_t timing_blob12[12];
        static uint8_t timing_blob20[20];

        switch (state) {
        case 0:
            if (data == 0xAA) {
                state = 1;
            }
            break;
        case 1:
            cmd = data;
            if (cmd == 0x10) {
                state = 10;
            } else if (cmd == 0x11) {
                state = 20;
            } else if (cmd == 0x12) {
                state = 40;
            } else if (cmd == 0x13) {
                state = 60;
            } else if (cmd >= 0x01 && cmd <= 0x04) {
                state = 2;
            } else {
                state = 0;
            }
            break;
        case 2:
            if (data == 0x55) {
                rx_cmd_val = cmd;
                rx_cmd_ready = 1;
            }
            state = 0;
            break;
        case 10:
            rx_screen_res = data;
            state = 11;
            break;
        case 11:
            rx_lvds_mode = data;
            state = 12;
            break;
        case 12:
            rx_screen_pattern = data;
            state = 13;
            break;
        case 13:
            if (data == 0x55) {
                rx_cmd_val = 0x10;
                rx_cmd_ready = 1;
            }
            state = 0;
            break;
        case 20:
            timing_blob8[0] = data;
            state = 21;
            break;
        case 21:
            timing_blob8[1] = data;
            state = 22;
            break;
        case 22:
            timing_blob8[2] = data;
            state = 23;
            break;
        case 23:
            timing_blob8[3] = data;
            state = 24;
            break;
        case 24:
            timing_blob8[4] = data;
            state = 25;
            break;
        case 25:
            timing_blob8[5] = data;
            state = 26;
            break;
        case 26:
            timing_blob8[6] = data;
            state = 27;
            break;
        case 27:
            timing_blob8[7] = data;
            state = 28;
            break;
        case 28:
            if (data == 0x55) {
                rx_timing_ha = (uint16_t)timing_blob8[0] | ((uint16_t)timing_blob8[1] << 8);
                rx_timing_va = (uint16_t)timing_blob8[2] | ((uint16_t)timing_blob8[3] << 8);
                rx_timing_fps_x10 = (uint16_t)timing_blob8[4] | ((uint16_t)timing_blob8[5] << 8);
                rx_timing_pclk_hz = 0;
                rx_timing_lvds = timing_blob8[6];
                rx_timing_pattern = timing_blob8[7];
                rx_cmd_val = 0x11;
                rx_cmd_ready = 1;
            }
            state = 0;
            break;
        case 40:
            timing_blob12[0] = data;
            state = 41;
            break;
        case 41:
            timing_blob12[1] = data;
            state = 42;
            break;
        case 42:
            timing_blob12[2] = data;
            state = 43;
            break;
        case 43:
            timing_blob12[3] = data;
            state = 44;
            break;
        case 44:
            timing_blob12[4] = data;
            state = 45;
            break;
        case 45:
            timing_blob12[5] = data;
            state = 46;
            break;
        case 46:
            timing_blob12[6] = data;
            state = 47;
            break;
        case 47:
            timing_blob12[7] = data;
            state = 48;
            break;
        case 48:
            timing_blob12[8] = data;
            state = 49;
            break;
        case 49:
            timing_blob12[9] = data;
            state = 50;
            break;
        case 50:
            timing_blob12[10] = data;
            state = 51;
            break;
        case 51:
            timing_blob12[11] = data;
            state = 52;
            break;
        case 52:
            if (data == 0x55) {
                rx_timing_ha = (uint16_t)timing_blob12[0] | ((uint16_t)timing_blob12[1] << 8);
                rx_timing_va = (uint16_t)timing_blob12[2] | ((uint16_t)timing_blob12[3] << 8);
                rx_timing_fps_x10 = (uint16_t)timing_blob12[4] | ((uint16_t)timing_blob12[5] << 8);
                rx_timing_pclk_hz = (uint32_t)timing_blob12[6]
                                  | ((uint32_t)timing_blob12[7] << 8)
                                  | ((uint32_t)timing_blob12[8] << 16)
                                  | ((uint32_t)timing_blob12[9] << 24);
                rx_timing_lvds = timing_blob12[10];
                rx_timing_pattern = timing_blob12[11];
                rx_cmd_val = 0x12;
                rx_cmd_ready = 1;
            }
            state = 0;
            break;
        case 60: timing_blob20[0] = data; state = 61; break;
        case 61: timing_blob20[1] = data; state = 62; break;
        case 62: timing_blob20[2] = data; state = 63; break;
        case 63: timing_blob20[3] = data; state = 64; break;
        case 64: timing_blob20[4] = data; state = 65; break;
        case 65: timing_blob20[5] = data; state = 66; break;
        case 66: timing_blob20[6] = data; state = 67; break;
        case 67: timing_blob20[7] = data; state = 68; break;
        case 68: timing_blob20[8] = data; state = 69; break;
        case 69: timing_blob20[9] = data; state = 70; break;
        case 70: timing_blob20[10] = data; state = 71; break;
        case 71: timing_blob20[11] = data; state = 72; break;
        case 72: timing_blob20[12] = data; state = 73; break;
        case 73: timing_blob20[13] = data; state = 74; break;
        case 74: timing_blob20[14] = data; state = 75; break;
        case 75: timing_blob20[15] = data; state = 76; break;
        case 76: timing_blob20[16] = data; state = 77; break;
        case 77: timing_blob20[17] = data; state = 78; break;
        case 78: timing_blob20[18] = data; state = 79; break;
        case 79: timing_blob20[19] = data; state = 80; break;
        case 80:
            if (data == 0x55) {
                rx_timing_ha = (uint16_t)timing_blob20[0] | ((uint16_t)timing_blob20[1] << 8);
                rx_timing_va = (uint16_t)timing_blob20[2] | ((uint16_t)timing_blob20[3] << 8);
                rx_timing_fps_x10 = (uint16_t)timing_blob20[4] | ((uint16_t)timing_blob20[5] << 8);
                rx_timing_pclk_hz = (uint32_t)timing_blob20[6]
                                  | ((uint32_t)timing_blob20[7] << 8)
                                  | ((uint32_t)timing_blob20[8] << 16)
                                  | ((uint32_t)timing_blob20[9] << 24);
                rx_timing_hbp = (uint16_t)timing_blob20[10] | ((uint16_t)timing_blob20[11] << 8);
                rx_timing_hfp = (uint16_t)timing_blob20[12] | ((uint16_t)timing_blob20[13] << 8);
                rx_timing_vbp = (uint16_t)timing_blob20[14] | ((uint16_t)timing_blob20[15] << 8);
                rx_timing_vfp = (uint16_t)timing_blob20[16] | ((uint16_t)timing_blob20[17] << 8);
                rx_timing_lvds = timing_blob20[18];
                rx_timing_pattern = timing_blob20[19];
                rx_cmd_val = 0x13;
                rx_cmd_ready = 1;
            }
            state = 0;
            break;
        default:
            state = 0;
            break;
        }
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
