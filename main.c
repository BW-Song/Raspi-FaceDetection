#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "highgui.h"
#include <sys/time.h>

#include "iothub.h"
#include "iothub_device_client.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "iothubtransportmqtt.h"

#include "FaceAPI.c"

// struct Content {
// 	char *buffer;
// 	size_t size;
// };

IOTHUB_DEVICE_CLIENT_HANDLE device_handle;
static char msgText[1024] = "SendMail = \"true\"";
char *groupName, *displayName, *lastName, *confidence, *updateTime;
char subscriptionKey[33];

static void connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
	(void)reason;
	(void)user_context;
	// This sample DOES NOT take into consideration network outages.
	if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
	{
		(void)printf("The device client is connected to iothub\r\n");
	}
	else
	{
		(void)printf("The device client has been disconnected\r\n");
	}
}

static void send_confirm_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
	(void)userContextCallback;
	(void)printf("Confirmation callback received with result %s\r\n", MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}

static void send_message(IOTHUB_DEVICE_CLIENT_HANDLE handle, char* message)
{
	IOTHUB_MESSAGE_HANDLE message_handle = IoTHubMessage_CreateFromString(message);
	// Set system properties
	(void)IoTHubMessage_SetMessageId(message_handle, "MSG_ID");
	(void)IoTHubMessage_SetCorrelationId(message_handle, "CORE_ID");
	(void)IoTHubMessage_SetContentTypeSystemProperty(message_handle, "application%2fjson");
	(void)IoTHubMessage_SetContentEncodingSystemProperty(message_handle, "utf-8");

	MAP_HANDLE propMap = IoTHubMessage_Properties(message_handle);
	IoTHubDeviceClient_SendEventAsync(handle, message_handle, send_confirm_callback, NULL);

	IoTHubMessage_Destroy(message_handle);
	printf("Message Sent\n");
}

static int device_method_callback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback)
{
    // Chiller *chiller = (Chiller *)userContextCallback;
    int result;
	char* re = (char*)malloc(20);
    if (strcmp("who", method_name) == 0)
	{
		(void)printf("Last Person Identified: %s\n",lastName);
		sprintf(re,"\"%s\"",lastName);
	}
	else if (strcmp("confidence", method_name) == 0)
	{
		(void)printf("The confidence is: %s\n",confidence);
		sprintf(re,"\"%s\"",confidence);
	}
	else if (strcmp("time", method_name) == 0)
	{
		(void)printf("Last identify time is: %s\n",updateTime);
		sprintf(re,"\"%s\"",updateTime);
	}
	else if (strcmp("train", method_name) == 0)
	{
		printf("lastName: %s\n",lastName);
		if (strcmp("None", lastName) == 0) (void)printf("Call Error, no person detected\n");
		else
		{
			(void)printf("Train\n");
			sprintf(re,"\"train\"");
			char* name = (char*)malloc(5);
			strncpy(name,&(payload)[9],4);
			name[4]='\0';
			//printf("%s\n",name);

			printf("Trained!\n");
		}
	}
    else
	{
		(void)printf("***No Method Recognized***\n");
		re = "\"No Method Recognized\"";
	}
	*response_size = strlen(re)+1;
	*response = (char*)strdup(re);
    return result;
}

int main()
{
    (void)printf("Face Detection Example\n");

    CvCapture* cap = cvCaptureFromCAM(0);
    IplImage* frame;

    struct timeval curTime;
	struct timeval lastTime;
	struct timeval sendTime;
	gettimeofday(&lastTime, NULL);
	gettimeofday(&sendTime, NULL);
    
    FILE* cs;
	cs = fopen("/home/pi/Connection.txt","r");
	char connectionString[128];
	fgets(connectionString, 128, cs);
	fclose(cs);
	// printf("Connection String: %s\n", connectionString);
	
	FILE* cs2;
    cs2 = fopen("/home/pi/Subscription.txt","r");
    fgets(subscriptionKey, 33, cs2);
    fclose(cs2);
	// printf("Subscription Key: %s\n", subscriptionKey);
	
    (void)IoTHub_Init();
	device_handle = IoTHubDeviceClient_CreateFromConnectionString(connectionString, MQTT_Protocol);
    if (device_handle == NULL)
	{
		(void)printf("Failure creating Iothub device.  Hint: Check you connection string.\r\n");
        IoTHub_Deinit();
        return 0;
	}
    (void)IoTHubDeviceClient_SetConnectionStatusCallback(device_handle, connection_status_callback, NULL);
    (void)IoTHubDeviceClient_SetDeviceMethodCallback(device_handle, device_method_callback, NULL);

    bool flag = true;
    lastName = (char*)malloc(5);
	confidence = (char*)malloc(8);
	updateTime = (char*)malloc(20);

    while (1)
    {
        frame = cvQueryFrame(cap);
        gettimeofday(&curTime, NULL);

        if (curTime.tv_sec - lastTime.tv_sec > 60 || flag)
        {
            cvSaveImage("/home/pi/cur.jpg",frame,0);
            flag = false;
			gettimeofday(&lastTime, NULL);

            FILE *fp;
			unsigned char* file_data;
			long length;
			fp= fopen("/home/pi/cur.jpg","rb");
            fseek(fp, 0, SEEK_END);         
			length= ftell(fp);           
			fseek(fp, 0, SEEK_SET);           
			file_data= (unsigned char *)malloc((length+1)*sizeof(unsigned char));
			fread(file_data, length, sizeof(unsigned char), fp);
            
            struct Content msg;
			msg.buffer = file_data;
			msg.size = (size_t)length;

            char* faceID = (char*)malloc(100);
            detect(subscriptionKey,msg,faceID);
            if (strstr(faceID, "notfound") == NULL)
            {
                char* personID = (char*)malloc(100);
                confidence = identify(subscriptionKey,"bwsong",faceID,personID);
                if (personID != NULL) lastName = checkName(subscriptionKey,"bwsong",personID);
                else if (strcmp("Else", lastName)) cvSaveImage("/home/pi/1.jpg",frame,0);
                else lastName = "None";
            }
            else
            {
                lastName = "None";
				confidence = "0.00000";
            }
            memset(updateTime,0,20);
			sprintf(updateTime,"%ld.%06d",curTime.tv_sec,curTime.tv_usec);
			printf("%s,%s,%s\n",lastName,confidence,updateTime);
            if (lastName == "None" && curTime.tv_sec - sendTime.tv_sec > 10)
			{
				printf("Send Mail\n");
				IoTHubDeviceClient_Destroy(device_handle);
				gettimeofday(&sendTime, NULL);
				device_handle = IoTHubDeviceClient_CreateFromConnectionString(connectionString, MQTT_Protocol);
				(void)IoTHubDeviceClient_SetConnectionStatusCallback(device_handle, connection_status_callback, NULL);
   				(void)IoTHubDeviceClient_SetDeviceMethodCallback(device_handle, device_method_callback, NULL);
			}
        }
    }
    cvReleaseCapture(&cap);
	curl_global_cleanup();
	IoTHubDeviceClient_Destroy(device_handle);
	IoTHub_Deinit();
	return 0;
}