#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>

#define PORT 8080
#define API_KEY "890eb485e44d3537114292f3c91dbba0"

void get_live_weather(const char* city, char* result) {
    int sock;
    struct hostent *server;
    struct sockaddr_in serv_addr;
    char request[512], buffer[4096];

    server = gethostbyname("api.openweathermap.org");
    if (server == NULL) {
        strcpy(result, "Blad DNS");
        return;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        strcpy(result, "API Timeout");
        close(sock);
        return;
    }

    sprintf(request, "GET /data/2.5/weather?q=%s&appid=%s&units=metric HTTP/1.1\r\nHost: api.openweathermap.org\r\nConnection: close\r\n\r\n", city, API_KEY);
    write(sock, request, strlen(request));

    int n = read(sock, buffer, 4095);
    if (n > 0) {
        buffer[n] = '\0';
        if (strstr(buffer, "401")) {
            strcpy(result, "Klucz API w trakcie aktywacji");
        } else {
            char *temp_pos = strstr(buffer, "\"temp\":");
            if (temp_pos) {
                float temp;
                sscanf(temp_pos + 7, "%f", &temp);
                sprintf(result, "%.1f C", temp);
            } else {
                strcpy(result, "Blad parsowania danych");
            }
        }
    } else {
        strcpy(result, "Brak odpowiedzi");
    }
    close(sock);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1, addrlen = sizeof(address);
    char buffer[2048] = {0};

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    printf("=========================================\n");
    printf(" Serwer Pogodowy - Hubert Luszczew\n");
    printf(" Data uruchomienia: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    printf(" Port: %d\n", PORT);
    printf("=========================================\n");
    fflush(stdout);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    while(1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        memset(buffer, 0, 2048);
        read(new_socket, buffer, 2048);

        char *city_param = strstr(buffer, "city=");
        char selected_city[50] = "Lublin", weather_info[100];
        if (city_param) sscanf(city_param, "city=%49[^& ]", selected_city);
        
        get_live_weather(selected_city, weather_info);

        char response[4096];
        int len = sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\n\n"
            "<html><head><title>Pogoda LIVE</title></head>"
            "<body style='font-family: Arial, sans-serif; text-align: center; background: #f0f2f5; padding: 50px;'>"
            "<div style='background: white; display: inline-block; padding: 30px; border-radius: 15px; shadow: 0 4px 8px rgba(0,0,0,0.1);'>"
            "<h2>🌍 System Pogodowy v1.0</h2>"
            "<p style='color: #666;'>Autor: Hubert Luszczew</p><hr>"
            "<form action='/'> Lokalizacja: "
            "<select name='city' style='padding: 5px; border-radius: 5px;'>"
            "<optgroup label='Polska'><option>Lublin</option><option>Warszawa</option><option>Krakow</option></optgroup>"
            "<optgroup label='Europa'><option>Berlin</option><option>Paris</option><option>Madrid</option><option>Rome</option></optgroup>"
            "</select> <input type='submit' value='Sprawdz' style='padding: 5px 15px; cursor: pointer;'></form>"
            "<h3>Miasto: %s</h3><h1 style='color: #0056b3;'>%s</h1>"
            "<p style='font-size: 0.8em; color: #999;'>Ostatnia aktualizacja: %02d:%02d:%02d</p>"
            "</div></body></html>", selected_city, weather_info, tm.tm_hour, tm.tm_min, tm.tm_sec);
        
        write(new_socket, response, len);
        close(new_socket);
    }
    return 0;
}