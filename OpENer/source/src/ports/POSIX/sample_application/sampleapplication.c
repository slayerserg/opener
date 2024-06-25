/*******************************************************************************
 * Copyright (c) 2012, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
 /*
 connection 101 100  
 connection 254 100
 connection 101 254
 */
#define _BSD_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

#include "opener_api.h"
#include "appcontype.h"
#include "trace.h"
#include "cipidentity.h"

#include "ciptcpipinterface.h"
#include "cipqos.h"
//#include "nvdata.h"
#if defined(OPENER_ETHLINK_CNTRS_ENABLE) && 0 != OPENER_ETHLINK_CNTRS_ENABLE
  #include "cipethernetlink.h"
  #include "ethlinkcbs.h"
#endif

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>

#define GFL_ASSEMBLY_OBJECT                        100 //0x064
#define TCMS_ASSEMBLY_OBJECT                       101 //0x065
#define HEARTBEAT_ASSEMBLY_OBJECT                  254 //0X0FE
#define APP_CONFIG_1                                 1 //0x001
#define DEMO_APP_EXPLICT_ASSEMBLY_NUM              154 //0x09A


#define UDS_SERVER_ADDRESS "/etc/uds_socket"
#define GFL_ASSEMBLY_OBJECT_SIZE 1480
#define TCMS_ASSEMBLY_OBJECT_SIZE 1272
#define HEARTBEAT_ASSEMBLY_OBJECT_SIZE 32
#define DIVIDER 8
#define THREAD_SLEEP_MICROSECONDS 1
#define HEADER_LENGTH 4
#define IMALIVE_LENGTH 1


pthread_mutex_t uds_lock;

#if defined (GFL_ASSEMBLY_OBJECT) && defined (TCMS_ASSEMBLY_OBJECT)
EipUint8 data_from_python_application[(GFL_ASSEMBLY_OBJECT_SIZE / DIVIDER) - HEADER_LENGTH] = \
  {0x43, 0x2c, 0xbb, 0xe7, 0xc0, 0x2c, 0x3b, 0x8e, 0x22, 0x4b, 0x9d, 0x9e, 0xcf, 0x1a, 0xc5, 0x39, \
   0x7c, 0x9f, 0xde, 0xf8, 0x32, 0xe,  0x9b, 0x7a, 0x9,  0x5d, 0xf5, 0xb3, 0x36, 0xc8, 0xa4, 0x10, \
   0x3a, 0xd3, 0xe2, 0x89, 0x63, 0xec, 0xec, 0xd9, 0xbb, 0x5,  0x4d, 0x46, 0xb5, 0x97, 0xfe, 0x47, \
   0xa7, 0xc9, 0x8b, 0x2c, 0x80, 0x97, 0x94, 0x90, 0xbc, 0xeb, 0x1b, 0x7b, 0xdd, 0xb9, 0xef, 0xd2, \
   0xda, 0x87, 0xe3, 0x56, 0x1f, 0xf5, 0x3,  0x17, 0x82, 0xc9, 0x46, 0x99, 0xb,  0x6d, 0xbb, 0xb7, \
   0x85, 0x0,  0x15, 0xc8, 0xfb, 0xf6, 0x3a, 0x38, 0x58, 0xe4, 0x9,  0x4c, 0x7f, 0xea, 0x15, 0x78, \
   0x43, 0x34, 0x44, 0x67, 0x5e, 0xee, 0x4e, 0xab, 0xe4, 0x3e, 0xd4, 0xb4, 0xcc, 0x9b, 0xdb, 0xdf, \
   0xd8, 0x65, 0x81, 0xcd, 0x18, 0x18, 0x55, 0xa4, 0xb9, 0x5e, 0x22, 0x5,  0x54, 0x7,  0xba, 0xb,  \
   0x27, 0x1f, 0xdb, 0xf1, 0x5f, 0xab, 0x9b, 0x7f, 0xd6, 0xe8, 0x92, 0xc1, 0xca, 0x27, 0xa4, 0x7f, \
   0x46, 0x3d, 0x25, 0xdb, 0x4a, 0xd7, 0x57, 0x64, 0xa6, 0x65, 0x1a, 0xcb, 0xa1, 0xe4, 0x80, 0x70, \
   0x6c, 0xb0, 0x43, 0x98, 0xea, 0xc3, 0x1f, 0xf8, 0x9a, 0xf6, 0x70, 0xa2, 0xc8, 0xb1, 0x67, 0xf1, \
   0x77, 0x69, 0x5f, 0x7d, 0x20};
EipUint8 gfl_assembly_object_data[GFL_ASSEMBLY_OBJECT_SIZE / DIVIDER]; /* Gapfiller assembly object */
EipUint8 tcms_assembly_object_data[TCMS_ASSEMBLY_OBJECT_SIZE / DIVIDER]; /* TCMS/MPU assembly object */
#endif

EipUint8 g_assembly_data_config_1[10]; /* Demo Config */
EipUint8 g_assembly_data_explicit[32]; /* Explicit */

bool firstConn = true;



void trace_timestamp(char *args, int value)
{
  time_t timer;
  char buffer[26];
  struct tm* tm_info;
  timer = time(NULL);
  tm_info = localtime(&timer);
  strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  OPENER_TRACE_INFO("%s - %s %d\n", buffer, args, value);
}

// int socket_fd()
// {
//     struct sockaddr_un socket_address;

//     int sfd = socket(AF_UNIX, SOCK_STREAM, 0);

//     if (sfd == -1) {
//       puts("Bad socket file");
//       exit(0);
//     }

//     memset(&socket_address, 0, sizeof(struct sockaddr_un));
//     socket_address.sun_family = AF_UNIX;
//     strncpy(socket_address.sun_path, UDS_SERVER_ADDRESS, sizeof(socket_address.sun_path) - 1);

//     if (connect(sfd, (struct sockaddr *) &socket_address,
//                 sizeof(struct sockaddr_un)) == -1) {
//       puts("Can not connect with socket");
//       exit(0);
//     }
//     return sfd;
// }

/* local functions */

void *uds_thread(void *arg) {

    //int sfd = socket_fd();

    #if defined (GFL_ASSEMBLY_OBJECT) && defined (TCMS_ASSEMBLY_OBJECT)
    while (1) {

        
        //send(sfd, tcms_assembly_object_data, sizeof(tcms_assembly_object_data), 0);
        //trace_timestamp("Sending assembly: ", (int)tcms_assembly_object_data[0]);
        //OPENER_TRACE_INFO("\nValue at 0 gfl: %d | address: %p:", *(gfl_assembly_object_data), (void *)&gfl_assembly_object_data);
        //OPENER_TRACE_INFO("\nValue at 0 python: %d | address: %p:", *(data_from_python_application), (void *)&data_from_python_application);
        
        pthread_mutex_lock(&uds_lock);
        //memset(data_from_python_application, 0, sizeof(data_from_python_application));
        //recv(sfd, data_from_python_application, GFL_ASSEMBLY_OBJECT_SIZE, 0);
        memcpy( &gfl_assembly_object_data[HEADER_LENGTH], &data_from_python_application ,
              sizeof(data_from_python_application));
        pthread_mutex_unlock(&uds_lock);

        usleep(THREAD_SLEEP_MICROSECONDS);
    }
    #endif
}

void AppendGFLHeaderToData()
{
  gfl_assembly_object_data[0] = 1;
  gfl_assembly_object_data[1] = 0;
  gfl_assembly_object_data[2] = 0;
  gfl_assembly_object_data[3] = 0;
}

void ResetHeartbeat()
{
  gfl_assembly_object_data[4] = 0;
}

static EipUint8 hb = 0;

void UpdateHeartBeat()
{
  hb++;
  gfl_assembly_object_data[4] = hb;
}

void updateBytes()
{
  UpdateHeartBeat();
}

EipStatus ApplicationInitialization(void) {
  /* create 3 assembly object instances*/
  pthread_mutex_init(&uds_lock, NULL);

  pthread_t id_uds;
  pthread_create(&id_uds, NULL, uds_thread, NULL);
  memset((void *)&gfl_assembly_object_data, 0, sizeof(unsigned short)*6);
  AppendGFLHeaderToData();
  ResetHeartbeat();


  #if defined (GFL_ASSEMBLY_OBJECT) && defined (TCMS_ASSEMBLY_OBJECT)
    /*CONFIG*/
  CreateAssemblyObject( APP_CONFIG_1, g_assembly_data_config_1,
                        sizeof(g_assembly_data_config_1) );

  /*TCMS/MPU*/
  CreateAssemblyObject( TCMS_ASSEMBLY_OBJECT, tcms_assembly_object_data,
                        sizeof(tcms_assembly_object_data) );

  /*HEARTBEAT*/
  CreateAssemblyObject( HEARTBEAT_ASSEMBLY_OBJECT, NULL, 0);

  /*Gapfill*/
  CreateAssemblyObject( GFL_ASSEMBLY_OBJECT, gfl_assembly_object_data,
                        sizeof(gfl_assembly_object_data) );

  ConfigureExclusiveOwnerConnectionPoint(0, TCMS_ASSEMBLY_OBJECT ,
                                        GFL_ASSEMBLY_OBJECT,
					                              APP_CONFIG_1);

  ConfigureInputOnlyConnectionPoint(0, HEARTBEAT_ASSEMBLY_OBJECT,
                                        GFL_ASSEMBLY_OBJECT,
					                              APP_CONFIG_1);
  
  ConfigureInputOnlyConnectionPoint(1, TCMS_ASSEMBLY_OBJECT,
                                        HEARTBEAT_ASSEMBLY_OBJECT,
                                        APP_CONFIG_1);


  #endif

  return kEipStatusOk;
}

void HandleApplication(void) {
  /* check if application needs to trigger an connection */
}

void CheckIoConnectionEvent(unsigned int output_assembly_id,
                            unsigned int input_assembly_id,
                            IoConnectionEvent io_connection_event) {
  /* maintain a correct output state according to the connection state*/

  (void) output_assembly_id; /* suppress compiler warning */
  (void) input_assembly_id; /* suppress compiler warning */
  (void) io_connection_event; /* suppress compiler warning */
}

EipStatus AfterAssemblyDataReceived(CipInstance *instance) {
  EipStatus status = kEipStatusOk;

  OPENER_TRACE_INFO("\ninstance->instance_number is %d\n", instance->instance_number);

  /*handle the data received e.g., update outputs of the device */
  switch (instance->instance_number) {
     case TCMS_ASSEMBLY_OBJECT:
      //OPENER_TRACE_INFO("\nTCMS_ASSEMBLY_OBJECT\n");
      status = kEipStatusOk;
      break;
    case HEARTBEAT_ASSEMBLY_OBJECT:
      OPENER_TRACE_INFO("\nHEARTBEAT_ASSEMBLY_OBJECT\n");
      status = kEipStatusOk;
      break;
    case DEMO_APP_EXPLICT_ASSEMBLY_NUM:
    OPENER_TRACE_INFO("Explicit at 0: %d", g_assembly_data_explicit[0]);
      /* do something interesting with the new data from
       * the explicit set-data-attribute message */
      break;
    case APP_CONFIG_1:
      /* Add here code to handle configuration data and check if it is ok
       * The demo application does not handle config data.
       * However in order to pass the test we accept any data given.
       * EIP_ERROR
       */
      status = kEipStatusOk;
      break;
    default:
      OPENER_TRACE_INFO(
        "Unknown assembly instance ind AfterAssemblyDataReceived");
      break;
  }
  return status;
}

EipBool8 BeforeAssemblyDataSend(CipInstance *pa_pstInstance) {
  /*update data to be sent e.g., read inputs of the device */
  /*In this sample app we mirror the data from out to inputs on data receive
   * therefore we need nothing to do here. Just return true to inform that
   * the data is new.
   */
  UpdateHeartBeat();
  // if (gfl_assembly_object_data[1] >= 3 && firstConn) // Reseting sequence byte after connection manager handshake
  //     {
  //       firstConn = false;
  //       gfl_assembly_object_data[0] = 0;
  //       gfl_assembly_object_data[1] = 1;
  //     }
  // else if (gfl_assembly_object_data[0] == 255)
  //     {
  //       gfl_assembly_object_data[0] = 0;
  //       gfl_assembly_object_data[1] = 0;
  //     }
  // else if (gfl_assembly_object_data[1] == 255)
  //     {
  //       gfl_assembly_object_data[1] = 0;
  //       gfl_assembly_object_data[0] += 1;
  //     }
  // else
  //     {
  //       gfl_assembly_object_data[1] += 1;
  //     }

  if (pa_pstInstance->instance_number == DEMO_APP_EXPLICT_ASSEMBLY_NUM) {
    /* do something interesting with the existing data
     * for the explicit get-data-attribute message */
  }
  return true;
}

EipStatus ResetDevice(void) {
  /* add reset code here*/
  CloseAllConnections();
  return kEipStatusOk;
}

EipStatus ResetDeviceToInitialConfiguration(void) {
  /*rest the parameters */
  g_encapsulation_inactivity_timeout = 120;
  /*than perform device reset*/
  ResetDevice();
  return kEipStatusOk;
}

void *
CipCalloc(size_t number_of_elements,
          size_t size_of_element) {
  return calloc(number_of_elements, size_of_element);
}

void CipFree(void *data) {
  free(data);
}

void RunIdleChanged(EipUint32 run_idle_value) {
  OPENER_TRACE_INFO("Run/Idle handler triggered\n");
  if( (0x0001 & run_idle_value) == 1 ) {
    CipIdentitySetExtendedDeviceStatus(kAtLeastOneIoConnectionInRunMode);
  } else {
    CipIdentitySetExtendedDeviceStatus(
      kAtLeastOneIoConnectionEstablishedAllInIdleMode);
  }
  (void) run_idle_value;
}

