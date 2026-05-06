#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

void get_weather(const char *city, char *result) {
    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd;
    char request[512], buffer[4096];
    char *api_key = getenv("WEATHER_API_KEY");

    if (api_key == NULL) {
        strcpy(result, "Blad: Brak klucza API");
        fprintf(stderr, "Blad: Nie znaleziono zmiennej WEATHER_API_KEY\n");
        fflush(stderr);
        return;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server = gethostbyname("api.openweathermap.org");
    if (server == NULL) {
        strcpy(result, "Blad DNS");
        return;
    }
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        strcpy(result, "Blad polaczenia");
        close(sockfd);
        return;
    }
    
    sprintf(request, "GET /data/2.5/weather?q=%s&appid=%s&units=metric HTTP/1.1\r\nHost: api.openweathermap.org\r\nConnection: close\r\n\r\n", city, api_key);
    write(sockfd, request, strlen(request));
    
    int n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        char *temp_ptr = strstr(buffer, "\"temp\":");
        if (temp_ptr) {
            float temp;
            sscanf(temp_ptr + 7, "%f", &temp);
            sprintf(result, "%.1f C", temp);
        } else {
            strcpy(result, "Brak danych");
        }
    }
    close(sockfd);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    while(1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        read(new_socket, buffer, 1024);

        char selected_city[50] = "Lublin";
        char *city_query = strstr(buffer, "city=");
        if (city_query) {
            sscanf(city_query + 5, "%[^& ]", selected_city);
        }

        char weather_info[50];
        get_weather(selected_city, weather_info);

        printf("Zapytanie: %s -> %s\n", selected_city, weather_info);
        fflush(stdout);

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        char response[4096];
        int len = sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\n\n"
            "<html><head><title>Pogoda</title></head>"
            "<body style='font-family: Arial, sans-serif; text-align: center; background: #f0f2f5; padding: 50px;'>"
            "<div style='background: white; display: inline-block; padding: 30px; border-radius: 15px; box-shadow: 0 4px 8px rgba(0,0,0,0.1);'>"
            "<h2>Pogoda</h2>"
            "<p style='color: #666;'>Autor: Hubert Luszczew</p><hr style='border: 0; border-top: 1px solid #eee; margin: 20px 0;'>"
            "<form action='/'> Lokalizacja: "
            "<select name='city' style='padding: 5px; border-radius: 5px; border: 1px solid #ccc;'>"
                "<optgroup label='Polska'>"
                    "<option>Lublin</option><option>Warszawa</option><option>Krakow</option>"
                "</optgroup>"
                "<optgroup label='Niemcy'>"
                    "<option>Berlin</option><option>Hamburg</option><option>Munich</option>"
                "</optgroup>"
                "<optgroup label='Francja'>"
                    "<option>Paris</option><option>Marseille</option><option>Lyon</option>"
                "</optgroup>"
                "<optgroup label='Hiszpania'>"
                    "<option>Madrid</option><option>Barcelona</option><option>Sevilla</option>"
                "</optgroup>"
                "<optgroup label='Wlochy'>"
                    "<option>Rome</option><option>Milan</option><option>Naples</option>"
                "</optgroup>"
            "</select> "
            "<input type='submit' value='Sprawdz' style='padding: 5px 15px; cursor: pointer; border-radius: 5px; border: 1px solid #ccc; background: #fff;'>"
            "</form>"
            "<h3 style='margin-top: 30px;'>Miasto: %s</h3>"
            "<h1 style='color: #0056b3; font-size: 3em; margin: 10px 0;'>%s</h1>"
            "<p style='font-size: 0.8em; color: #999;'>Ostatnia aktualizacja: %02d:%02d:%02d</p>"
            "</div></body></html>", selected_city, weather_info, tm.tm_hour, tm.tm_min, tm.tm_sec);

        write(new_socket, response, len);
        close(new_socket);
    }
    return 0;
}
