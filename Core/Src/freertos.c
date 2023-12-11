/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <uxr_transport.h>
#include "uxr/client/transport.h"
#include "uxr/client/client.h"
#include "HelloWorld.h"
#include "usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STREAM_HISTORY  4
#define BUFFER_SIZE     UXR_CONFIG_CUSTOM_TRANSPORT_MTU*STREAM_HISTORY

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint8_t output_reliable_stream_buffer[BUFFER_SIZE];
uint8_t input_reliable_stream_buffer[BUFFER_SIZE];
const char* participant_xml = "<dds>"
        "<participant>"
        "<rtps>"
        "<name>default_xrce_participant</name>"
        "</rtps>"
        "</participant>"
        "</dds>";
const char* topic_xml = "<dds>"
        "<topic>"
        "<name>HelloWorldTopic</name>"
        "<dataType>HelloWorld</dataType>"
        "</topic>"
        "</dds>";
const char* datawriter_xml = "<dds>"
        "<data_writer>"
        "<topic>"
        "<kind>NO_KEY</kind>"
        "<name>HelloWorldTopic</name>"
        "<dataType>HelloWorld</dataType>"
        "</topic>"
        "</data_writer>"
        "</dds>";
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 1024);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  printf("Hardware initialize OK.\r\n");
  uxrCustomTransport transport;

  uxr_set_custom_transport_callbacks(
      &transport,
      true,
      my_custom_transport_open,
      my_custom_transport_close,
      my_custom_transport_write,
      my_custom_transport_read);

  if (!uxr_init_custom_transport(&transport, NULL))
  {
	  while(1);
  }

  uxrSession session;
  uxr_init_session(&session, &transport.comm, 0x08ABCDEF);
  if (!uxr_create_session(&session))
  {
	  while(1);
  }
  // Streams
  
  uxrStreamId reliable_out = uxr_create_output_reliable_stream(&session, output_reliable_stream_buffer, BUFFER_SIZE,
                  STREAM_HISTORY);

  
  uxr_create_input_reliable_stream(&session, input_reliable_stream_buffer, BUFFER_SIZE, STREAM_HISTORY);

  // Create entities
  uxrObjectId participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);

  uint16_t participant_req = uxr_buffer_create_participant_xml(&session, reliable_out, participant_id, 0,
                  participant_xml, UXR_REPLACE);

  uxrObjectId topic_id = uxr_object_id(0x01, UXR_TOPIC_ID);

  uint16_t topic_req = uxr_buffer_create_topic_xml(&session, reliable_out, topic_id, participant_id, topic_xml,
                  UXR_REPLACE);

  uxrObjectId publisher_id = uxr_object_id(0x01, UXR_PUBLISHER_ID);
  const char* publisher_xml = "";
  uint16_t publisher_req = uxr_buffer_create_publisher_xml(&session, reliable_out, publisher_id, participant_id,
                  publisher_xml, UXR_REPLACE);

  uxrObjectId datawriter_id = uxr_object_id(0x01, UXR_DATAWRITER_ID);

  uint16_t datawriter_req = uxr_buffer_create_datawriter_xml(&session, reliable_out, datawriter_id, publisher_id,
                  datawriter_xml, UXR_REPLACE);

  // Send create entities message and wait its status
  uint8_t status[4];
  uint16_t requests[4] = {
      participant_req, topic_req, publisher_req, datawriter_req
  };
  if (!uxr_run_session_until_all_status(&session, 1000, requests, status, 4))
  {
      printf("Error at create entities: participant: %i topic: %i publisher: %i darawriter: %i\n", status[0],
              status[1], status[2], status[3]);
  }

  // Write topics
  uint32_t count = 0;
  /* Infinite loop */
  for(;;)
  {
      HelloWorld topic = {
          ++count, "Hello DDS world!"
      };

      ucdrBuffer ub;
      uint32_t topic_size = HelloWorld_size_of_topic(&topic, 0);
      uxr_prepare_output_stream(&session, reliable_out, datawriter_id, &ub, topic_size);
      HelloWorld_serialize_topic(&ub, &topic);

      printf("Send topic: %s, id: %li\n", topic.message, topic.index);
      uxr_run_session_time(&session, 1000);
  }
  // Delete resources
  uxr_delete_session(&session);
  uxr_close_custom_transport(&transport);
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
