# Etap 1: Budowanie aplikacji (Build Stage)
FROM maven:3.9-eclipse-temurin-25 AS build
WORKDIR /app

# Kopiujemy definicję zależności, aby wykorzystać cache Dockera
COPY pom.xml .
# Pobieramy zależności bez budowania kodu - przyspieszy to kolejne buildy
RUN mvn dependency:go-offline -B

# Kopiujemy kod źródłowy i budujemy plik JAR
COPY src ./src
RUN mvn clean package -DskipTests

# Etap 2: Środowisko uruchomieniowe (Runtime Stage)
# Używamy lekkiego obrazu JRE dla Javy 25 na bazie Alpine Linux
FROM eclipse-temurin:25-jre-alpine
WORKDIR /app

# Tworzymy użytkownika systemowego, aby nie uruchamiać aplikacji jako root (bezpieczeństwo)
RUN addgroup -S spring && adduser -S spring -G spring
USER spring:spring

# Kopiujemy tylko gotowy plik JAR z poprzedniego etapu
COPY --from=build /app/target/*.jar app.jar

# Optymalizacja JVM dla kontenerów:
# - MaxRAMPercentage: automatyczne dopasowanie pamięci heap do limitów kontenera
# - UseG1GC: nowoczesny garbage collector
ENV JAVA_OPTS="-XX:MaxRAMPercentage=75.0 -XX:+UseG1GC"

# Dokumentujemy port, na którym nasłuchuje Spring Boot
EXPOSE 8080

# Uruchomienie aplikacji
# Wykorzystujemy formę tablicową (exec form), aby poprawnie obsługiwać sygnały zamknięcia (SIGTERM)
ENTRYPOINT ["sh", "-c", "java $JAVA_OPTS -jar app.jar"]
