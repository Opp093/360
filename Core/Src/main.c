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

// --- 新增：上位机通信协议状态机定义 ---
typedef enum {
    STATE_WAIT_HEAD1 = 0, // 等待帧头 0xAA
    STATE_WAIT_HEAD2,     // 等待帧头 0x55
    STATE_WAIT_CMD,       // 等待命令字
    STATE_WAIT_LEN,       // 等待数据长度
    STATE_WAIT_DATA,      // 接收有效数据
    STATE_WAIT_CS,        // 等待校验和
    STATE_WAIT_TAIL       // 等待帧尾
} UART_RX_StateTypeDef;

// --- 修改：增加 volatile 防止被编译器“暗杀” ---
volatile uint8_t rx_byte;                 
volatile UART_RX_StateTypeDef rx_state = STATE_WAIT_HEAD1; 
volatile uint8_t rx_cmd = 0;              
volatile uint8_t rx_len = 0;              
volatile uint8_t rx_data[16];             
volatile uint8_t rx_data_cnt = 0;         
volatile uint8_t rx_checksum = 0;         
volatile uint8_t frame_ready_flag = 0;    

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
  // HAL_GPIO_WritePin(BL_PWM_GPIO_Port, BL_PWM_Pin, GPIO_PIN_SET);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

  // --- 新增：启动串口接收中断（每次接收1字节） ---
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);

  max96755_video_RMW_config(s_96755_video_RMW_config_1920_check_board, CONFIG_NUM_755_VIDEO_checkboard);
//  check_test();                           //调试看寄存器的

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // --- 新增：终极硬件监听陷阱 (放在 while(1) 的第一行) ---
    // 检查物理寄存器是否收到了电平变化，或者是否发生了溢出(ORE)
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET || 
        __HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE)  != RESET) 
    {
        // 强制读取底层寄存器，清理堆积的废料
        volatile uint32_t tmpsr = huart1.Instance->SR;
        volatile uint32_t tmpdr = huart1.Instance->DR;
        
        // 向PC发出尖叫：我的硬件引脚确实收到电平了！
        HAL_UART_Transmit(&huart1, (uint8_t*)"[DEBUG] Physical RX OK!\n", 24, 100);
    }
    // ----------------------------------------------------
    // --- 新增：处理上位机下发的屏幕配置参数 ---
    if (frame_ready_flag == 1)
    {
        frame_ready_flag = 0; 
        // 0x02 定义为上位机下发的“单次寄存器透传”指令，长度固定为4字节
        if (rx_cmd == 0x02 && rx_len == 4)
        {
            // --- 绝杀测试：只要串口校验通过，立刻回复PC！不管后面IIC死活 ---
            HAL_UART_Transmit(&huart1, (uint8_t*)"CMD_OK\n", 7, 100);

            max_96755_register_RMW_CFG_TYPE config_item;
            config_item.Address = (rx_data[0] << 8) | rx_data[1]; 
            config_item.mask = rx_data[2];                        
            config_item.DATA = rx_data[3];                        

            // 底层IIC发送
            max96755_video_RMW_config(&config_item, 1);
        }
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

// --- 新增：UART 硬件错误急救包（防锁死核心） ---
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // 1. 读取 SR 和 DR 寄存器，这是底层清除 ORE(溢出)、FE(帧错) 的强制手段
        volatile uint32_t tmp = huart->Instance->SR;
        tmp = huart->Instance->DR;
        (void)tmp; // 防止编译器报 warning
        
        // 2. 强行复位 HAL 库的接收状态机
        huart->RxState = HAL_UART_STATE_READY;
        huart->ErrorCode = HAL_UART_ERROR_NONE;
        
        // 3. 重新开启中断，满血复活
        HAL_UART_Receive_IT(&huart1, (uint8_t *)&rx_byte, 1);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // 1. 状态机流转逻辑
        switch (rx_state)
        {
            case STATE_WAIT_HEAD1:
                if (rx_byte == 0xAA) {
                    rx_state = STATE_WAIT_HEAD2;
                    rx_checksum = 0xAA; // 开始累加校验和
                }
                break;

            case STATE_WAIT_HEAD2:
                if (rx_byte == 0x55) {
                    rx_state = STATE_WAIT_CMD;
                    rx_checksum += rx_byte;
                } else {
                    rx_state = STATE_WAIT_HEAD1; // 格式错误，打回原形
                }
                break;

            case STATE_WAIT_CMD:
                rx_cmd = rx_byte;
                rx_checksum += rx_byte;
                rx_state = STATE_WAIT_LEN;
                break;

            case STATE_WAIT_LEN:
                rx_len = rx_byte;
                rx_checksum += rx_byte;
                rx_data_cnt = 0;
                if (rx_len > 0 && rx_len <= 16) {
                    rx_state = STATE_WAIT_DATA;
                } else if (rx_len == 0) {
                    rx_state = STATE_WAIT_CS; // 无数据，直接校验
                } else {
                    rx_state = STATE_WAIT_HEAD1; // 长度超限，复位
                }
                break;

            case STATE_WAIT_DATA:
                rx_data[rx_data_cnt++] = rx_byte;
                rx_checksum += rx_byte;
                if (rx_data_cnt >= rx_len) {
                    rx_state = STATE_WAIT_CS; // 数据收完，准备校验
                }
                break;

            case STATE_WAIT_CS:
                if (rx_byte == rx_checksum) {
                    rx_state = STATE_WAIT_TAIL; // 校验通过！
                } else {
                    rx_state = STATE_WAIT_HEAD1; // 校验失败，丢弃脏数据
                }
                break;

            case STATE_WAIT_TAIL:
                if (rx_byte == 0x0D || rx_byte == 0x0A) {
                    frame_ready_flag = 1; // 斩获完整一帧，立起Flag通知主循环
                }
                rx_state = STATE_WAIT_HEAD1; // 无论成败，为下一帧做准备
                break;
        }

        // 2. 再次开启中断，准备接收下一个字节
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
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
