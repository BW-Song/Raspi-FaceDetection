#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

struct string {
	char *ptr;
	size_t len;
};

struct Content {
	char *buffer;
	size_t size;
};

void init_string(struct string *s) {
	s->len = 0;
	s->ptr = malloc(s->len+1);
	if (s->ptr == NULL) {
		fprintf(stderr, "malloc() failed\n");
		exit(EXIT_FAILURE);
	}
	s->ptr[0] = '\0';
}

size_t responseCallback(void *ptr, size_t size, size_t nmemb, struct string *s){
	size_t new_len = s->len + size*nmemb;
	s->ptr = realloc(s->ptr, new_len+1);
	if (s->ptr == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr+s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}

static size_t ReadCallback(void *content, size_t size, size_t nmemb, void *userp){
	// printf("ReadCallback\n");
	struct Content *wt = (struct Content *)userp;
	size_t buffer_size = size*nmemb;

	if(wt->size) {
		// copy as much as possible from the source to the destination 
		size_t copy_this_much = wt->size;
		if(copy_this_much > buffer_size)
		  copy_this_much = buffer_size;
		memcpy(content, wt->buffer, copy_this_much);

		wt->buffer += copy_this_much;
		wt->size -= copy_this_much;
		// printf("%d\n",copy_this_much);
		return copy_this_much; // we copied this many bytes 
	}

	return 0; // no more data left to deliver 
}

void createPersonGroup(char* subscriptionKey, char* personGroupName, char* displayName)
{
    CURL *curl;
    CURLcode res;
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(res));
        printf("Global Init Error\n");
        return;
    }
    curl = curl_easy_init();

    char* url = (char*)malloc(200);
    sprintf(url,"https://westus.api.cognitive.microsoft.com/face/v1.0/persongroups/%s",personGroupName);

    char* display = (char*)malloc(200);
    sprintf(display,"{\"name\": \"%s\"}",displayName);

    char* sub = (char*)malloc(200);
    sprintf(sub,"Ocp-Apim-Subscription-Key: %s", subscriptionKey);

    if (curl)
    {
        struct string s;
		init_string(&s);

        struct curl_slist *header = NULL;
		header = curl_slist_append(header, "Content-Type: application/json");
        header = curl_slist_append(header, sub);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, display);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		res = curl_easy_perform(curl);

        if (res != CURLE_OK)
		{
			printf("POST failed. Error code: %d", res);
			return;
		}
        else
		{
			printf("Person Group %s Created!\n", personGroupName);
			free(s.ptr);
		}
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void createPerson(char* subscriptionKey, char* personGroupName, char* personName, char* personID)
{
    CURL *curl;
    CURLcode res;
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(res));
        printf("Global Init Error\n");
        return;
    }
    curl = curl_easy_init();

    char* url = (char*)malloc(200);
	sprintf(url,"https://westus.api.cognitive.microsoft.com/face/v1.0/persongroups/%s/persons", personGroupName);

    char* name = (char*)malloc(200);
    sprintf(name,"{\"name\": \"%s\"}",personName);

    char* sub = (char*)malloc(200);
    sprintf(sub,"Ocp-Apim-Subscription-Key: %s", subscriptionKey);

    if (curl)
    {
        struct string s;
		init_string(&s);

        struct curl_slist *header = NULL;
		header = curl_slist_append(header, "Content-Type: application/json");
        header = curl_slist_append(header, sub);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, name);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		res = curl_easy_perform(curl);

        if (res != CURLE_OK)
		{
			printf("POST failed. Error code: %d", res);
			return;
		}
        else
		{
			printf("Person %s Created!\n", personName);
            memset(personID,0,37);
            memcpy(personID,&(s.ptr)[13],36);
			free(s.ptr);
		}
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return;
}

void detect(char* subscriptionKey, struct Content msg, char* faceID)
{
    CURL *curl;
    CURLcode res;
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(res));
        printf("Global Init Error\n");
        return;
    }
    curl = curl_easy_init();

    char* sub = (char*)malloc(200);
    sprintf(sub,"Ocp-Apim-Subscription-Key: %s", subscriptionKey);

    if (curl)
	{
		struct string s;
		init_string(&s);

        struct curl_slist *header = NULL;
		header = curl_slist_append(header, "Content-Type: application/octet-stream");
        header = curl_slist_append(header, sub);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_URL, "https://westus.api.cognitive.microsoft.com/face/v1.0/detect");
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, msg.size);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadCallback);
		curl_easy_setopt(curl, CURLOPT_READDATA, &msg);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		res = curl_easy_perform(curl);

        if (res != CURLE_OK)
		{
			printf("POST failed. Error code: %d", res);
			return;
		}
        else
        {
            char* tar = strstr(s.ptr, "id");
            printf("%s\n",s.ptr);
            if (tar != NULL) 
            {
                strncpy(faceID,&(s.ptr)[12],36);
                faceID[36] = '\0';
                printf("Face Detected! ID: %s\n", faceID);
            }
            else
            {
                sprintf(faceID,"notfound\0");
                printf("No Face Detected!\n");
            }
            free(s.ptr);
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return;
}

char* identify(char* subscriptionKey, char* personGroupName, char* faceID, char* personID)
{
    char* confidence;
    CURL *curl;
    CURLcode res;
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(res));
        printf("Global Init Error\n");
        return NULL;
    }
    curl = curl_easy_init();

    char* name = (char*)malloc(200);
	sprintf(name,"{\"personGroupId\":\"%s\",\"faceIds\":[\"%s\"]}",personGroupName, faceID);

    char* sub = (char*)malloc(200);
    sprintf(sub,"Ocp-Apim-Subscription-Key: %s", subscriptionKey);

    if (curl)
    {
        struct string s;
		init_string(&s);

        struct curl_slist *header = NULL;
        header = curl_slist_append(header, "Content-Type: application/json");
		header = curl_slist_append(header, sub);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		curl_easy_setopt(curl, CURLOPT_URL, "https://westus.api.cognitive.microsoft.com/face/v1.0/identify");
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, name);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		res = curl_easy_perform(curl);

        if (res != CURLE_OK)
		{
			printf("POST failed. Error code: %d", res);
			return NULL;
		}
        else
        {
            memcpy(personID,&(s.ptr)[77],36);
			personID[36] = '\0';
            char* tar = strstr(s.ptr, "confidence");
			confidence = (char*)malloc(7);
            if (tar == NULL) sprintf(confidence,"0.00000\0");
			else 
            {
                strncpy(confidence,&(tar)[12],6);
                confidence[6] = '\0';
            }
            printf("personID: %s\n", personID);
            printf("Confidence: %s\n", confidence);
            free(s.ptr);
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return confidence;
}

char* checkName(char* subscriptionKey, char* personGroupName, char* personID)
{
    char* lastName;
    CURL *curl;
    CURLcode res;
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(res));
        printf("Global Init Error\n");
        return NULL;
    }
    curl = curl_easy_init();

    char* url = (char*)malloc(200);
	sprintf(url,"https://westus.api.cognitive.microsoft.com/face/v1.0/persongroups/%s/persons/%s", personGroupName, personID);

    char* sub = (char*)malloc(200);
    sprintf(sub,"Ocp-Apim-Subscription-Key: %s", subscriptionKey);

    if (curl)
	{
		struct string s;
		init_string(&s);

        struct curl_slist *header = NULL;
		header = curl_slist_append(header, sub);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
		curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		res = curl_easy_perform(curl);

        if (res != CURLE_OK)
		{
			printf("GET failed. Error code: %d", res);
			return NULL;
		}
		else
        {
            char* tar = strstr(s.ptr, "name");
            if (tar == NULL) lastName = "Else\0";
            else
            {
                lastName = (char*)malloc(5);
                strncpy(lastName,&(tar)[7],4);
                lastName[4] = '\0';
            }
            free(s.ptr);
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return lastName;
}

void addFace(char* subscriptionKey, char* personGroupName, char* personID, char* fileLocation)
{
    char* dest = (char*)malloc(100);
	int n = snprintf(dest,100,"/home/pi/FaceDetection/1/1.jpg");
	FILE *fp;
	unsigned char* file_data;
	long length;
	fp= fopen(dest,"rb");
	fseek(fp, 0, SEEK_END);         
	length= ftell(fp);           
	fseek(fp, 0, SEEK_SET);           
	file_data= (unsigned char *)malloc((length+1)*sizeof(unsigned char));
	fread(file_data, length, sizeof(unsigned char), fp);
	struct Content msg;
	msg.buffer = file_data;
	msg.size = (size_t)length;

    CURL *curl;
    CURLcode res;
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(res));
        printf("Global Init Error\n");
        return;
    }
    curl = curl_easy_init();

    char* url = (char*)malloc(200);
	sprintf(url,"https://westus.api.cognitive.microsoft.com/face/v1.0/persongroups/%s/persons/%s/persistedFaces",personGroupName,personID);

    char* sub = (char*)malloc(200);
    sprintf(sub,"Ocp-Apim-Subscription-Key: %s", subscriptionKey);

    if (curl)
    {
        struct string s;
		init_string(&s);

        struct curl_slist *header = NULL;
		header = curl_slist_append(header, "Content-Type: application/octet-stream");
		header = curl_slist_append(header, sub);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, msg.size);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadCallback);
		curl_easy_setopt(curl, CURLOPT_READDATA, &msg);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		res = curl_easy_perform(curl);

        if (res != CURLE_OK)
		{
			printf("POST failed. Error code: %d", res);
			return;
		}
        else
        {
            printf("Face Added!\n");
            free(s.ptr);
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return;
}

void train(char* subscriptionKey, char* personGroupName)
{
    CURL *curl;
    CURLcode res;
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(res));
        printf("Global Init Error\n");
        return;
    }
    curl = curl_easy_init();

    char* url = (char*)malloc(200);
	sprintf(url,"https://westus.api.cognitive.microsoft.com/face/v1.0/persongroups/%s/train",personGroupName);

    char* sub = (char*)malloc(200);
    sprintf(sub,"Ocp-Apim-Subscription-Key: %s", subscriptionKey);

    if (curl)
    {
        struct string s;
		init_string(&s);
		
		struct curl_slist *header = NULL;
		header = curl_slist_append(header, sub);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseCallback);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		res = curl_easy_perform(curl);

        if (res != CURLE_OK)
		{
			printf("POST failed. Error code: %d", res);
			return;
		}
        else
        {
            printf("Trained!\n");
            free(s.ptr);
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return;
}