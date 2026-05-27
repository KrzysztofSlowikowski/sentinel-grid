# Etap 1: Budowanie (kompilacja)
# Używamy nowszej wersji Go (1.24 nie istnieje jeszcze, użyjemy 1.23)
FROM golang:1.23-alpine AS builder

# Ustaw katalog roboczy wewnątrz kontenera
WORKDIR /app

# Kopiuj pliki zależności
COPY go.mod go.sum ./

# Pobierz zależności
RUN go mod download

# Kopiuj resztę kodu
COPY . .

# Skompiluj program (statycznie, żeby działał samodzielnie)
RUN CGO_ENABLED=0 GOOS=linux go build -o ingest .

# Etap 2: Obraz końcowy (mały)
FROM alpine:latest

# Zainstaluj certyfikaty SSL (potrzebne do TLS, na przyszłość)
RUN apk --no-cache add ca-certificates

# Kopiuj skompilowany program z etapu 1
COPY --from=builder /app/ingest /ingest

# Kopiuj .env (opcjonalne, można też przez zmienne środowiskowe)
COPY .env .env

# Port (nie jest używany bo Go słucha MQTT, nie HTTP, ale dla porządku)
EXPOSE 8080

# Uruchom program
CMD ["/ingest"]
