# Projekt Zespołowy (PZ)

Szybki start z wykorzystaniem Docker Compose.

## Uruchomienie aplikacji

1. Skopiuj plik środowiskowy (jeśli jeszcze tego nie zrobiłeś):
   ```bash
   cp .env.example .env
   ```
2. Uruchom kontenery:
   ```bash
   docker compose up -d
   ```
   *(Jeśli używasz starszej wersji: `docker-compose up -d`)*

3. Aby zresetować bazę danych (usunąć dane):
   ```bash
   docker compose down -v && docker compose up -d
   ```

## Linki do aplikacji

| Usługa | Adres URL | Opis |
| :--- | :--- | :--- |
| **Frontend Register** | [http://localhost:6769](http://localhost:6769) | Rejestracja użytkowników |
| **Admin Panel** | [http://localhost:6770](http://localhost:6770) | Zarządzanie systemem |
| **Backend API** | [http://localhost:8080](http://localhost:8080) | Główny serwer Spring Boot |
| **Swagger UI** | [http://localhost:8080/swagger-ui.html](http://localhost:8080/swagger-ui.html) | Dokumentacja API |

---
*Uwaga: Porty można zmienić w pliku `.env`.*
