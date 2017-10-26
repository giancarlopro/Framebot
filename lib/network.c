#include <telebot.h>

/* start curl in telebot_init */
void network_init(){
    curl_global_init(CURL_GLOBAL_DEFAULT);
}


MemStore * mem_store() {
    MemStore *mem = (MemStore *)malloc(sizeof(MemStore));

    mem->content = (char *)malloc(1);
    mem->size = 0;

    return mem;
}

void mem_store_free(MemStore * memStore) {
    if (memStore->content)
        free(memStore->content);
    
    free(memStore);
}

/* mem_write_callback is response of request Telegram */
size_t mem_write_callback(void *data, size_t size, size_t nmemb, void *userp) {
    size_t finalsz = size * nmemb;
    MemStore *json = (MemStore *)userp;

    json->content = (char *)realloc(json->content, json->size + finalsz + 1);

    if (json->content) {
        strcpy(json->content, (char *)data);
        json->size += finalsz;
        json->content[json->size] = '\0';

        return finalsz;
    }

    return 0;
}

/* send data to telegram */
MemStore * call_method(const char *token, char *method){
    size_t url_size = API_URL_LEN + strlen(token) + strlen(method) + 2;
    char * url = (char *)malloc(url_size);

    strcpy(url, API_URL);
    strcat(url, token);
    strcat(url, "/");
    strcat(url, method);

    url[url_size] = '\0';

    CURL * curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);


    MemStore *buff = mem_store();

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)buff);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mem_write_callback);

    free(url);

    if (curl_easy_perform(curl) == CURLE_OK) {
        return buff;
    }

    return NULL;
}

char * call_method_download(const char * token, char * dir, File *ofile){
    FILE * binary;
    size_t path_len, url_size; 
    char * namefile, *path, *url;

    url_size = API_URL_FILE_LEN + strlen(token) + strlen(ofile->file_path) + 2;
    url = (char *)malloc(url_size);

    strcpy(url, API_URL_FILE);
    strcat(url, token);
    strcat(url, "/");
    strcat(url, ofile->file_path);

    url[url_size] = '\0';

    CURL * curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    free(url);

    namefile = strstr(ofile->file_path, "/");
    namefile++;

    if(dir[strlen(dir) - 1] == '/')
        dir[strlen(dir) - 1] = '\0';

    if(!dir)
        path_len = strlen(namefile) + 2;
    else
        path_len = strlen(dir) + strlen(namefile) + 2;

    path = malloc(path_len);
    if(!path)
        return NULL;

    if(dir){
        strcpy(path, dir);
        strcat(path, "/");
        strcat(path, namefile);
    }
    else{
        strcpy(path, namefile);
    }

    binary = fopen(path, "wb");

    if(binary){
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)binary);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    }

    if (curl_easy_perform(curl) == CURLE_OK)
        return path;

    return NULL;
}

MemStore *call_method_wp(char *token, char *method, char *params) {
    size_t len = strlen(method) + strlen(params) + 1;
    char *tmp = (char *)malloc(len);

    MemStore *ms = call_method(token, tmp);

    free(tmp);

    return ms;
}
