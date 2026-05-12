Komenda budująca:

docker buildx build --platform linux/amd64,linux/arm64 `
  -t hubert00100/projekt-pogoda:latest `
  --ssh default `
  --cache-to type=registry,ref=hubert00100/projekt-pogoda:buildcache,mode=max `
  --cache-from type=registry,ref=hubert00100/projekt-pogoda:buildcache `
  --push .

![Logi budowania](buildx_build.png)
Wykorzystanie dedykowanego buildera opartego na sterowniku docker-container umożliwiło równoległe przeprowadzenie budowy dla architektur linux/amd64 oraz linux/arm64, co potwierdza nagłówek logów oraz końcowy zapis o utworzeniu wspólnej listy manifestów.

Proces budowania przebiegł z zachowaniem wysokich standardów bezpieczeństwa, o czym świadczą logi kroku [builder 6/7] dokumentujące użycie agenta SSH do bezpiecznego pobrania kodu źródłowego bezpośrednio z repozytorium GitHub.

Efektywność operacji została zwiększona dzięki zaawansowanemu zarządzaniu pamięcią podręczną w trybie max, co widać po statusach CACHED przy poszczególnych warstwach oraz finalnym eksporcie pełnych danych cache do zewnętrznego rejestru pod tagiem buildcache.


![Raport Scout](analiza_bledow.png)

Wynik skanowania potwierdza całkowity brak podatności, co udało się osiągnąć dzięki wykorzystaniu minimalistycznego obrazu bazowego typu scratch, pozbawionego zbędnych bibliotek i narzędzi systemowych. Bezpieczeństwo dodatkowo wzmacnia statyczna kompilacja aplikacji w języku C, która sprawia, że kontener zawiera wyłącznie niezbędny plik binarny, eliminując tym samym potencjalne drogi ataku.

Linki:

GitHub: 

https://github.com/hubert00100/docker_zad1

DockerHub: 

https://hub.docker.com/r/hubert00100/projekt-pogoda
