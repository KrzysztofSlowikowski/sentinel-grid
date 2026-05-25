package main //deklaracja do jakiego pakietu należy plik
//main to nazwa specjalna i oznacza że pakiet wykonywalny (program a nie biblioteka)

import ( //importujemy biblioteki/pakiety
    "database/sql" //1te to są biblioteki stantardowe, wbudowane w GO (interfejs do baz danych)
    "encoding/json" //(parsowanie JSON)
    "fmt" //(formatowanie I/O (print scan))
    "log" // (logowanie z timestampami)
    "strings" //(operacje ns stringach)
    "time" //(czas)

    mqtt "github.com/eclipse/paho.mqtt.golang" //2zewnętrzne pakiety (z githuba)
    _ "github.com/lib/pq" //3 te takie z podkreślnikiem to takie które importujemy ale nie używamy
)

// Konfiguracja (na razie hardkodowana, potem z pliku)
const ( //Zasada: Stałe mogą być tylko typami podstawowymi: string, liczba, boolean.
    MQTT_BROKER   = "tcp://localhost:1883"
    MQTT_CLIENTID = "go_ingest"

    DB_HOST     = "localhost"
    DB_PORT     = 5432
    DB_USER     = "postgres"
    DB_PASSWORD = "password"
    DB_NAME     = "sentinel"
)

// Struktura dla alertów (JSON)
type AlertMessage struct { //tworzymy nowy typ danych który będzie reprezentował wartość w alercie
    Type  string  `json:"type"` //`json:"type"` to tag -  informacja dla pakietu encoding/json jak mapować JSON
    Value float64 `json:"value,omitempty"`
    Z     int16   `json:"z,omitempty"` //omitempty = jeśli pole jest puste (0, "", nil), 
    // nie umieszczaj go w wyjściowym JSON.
}

// Połączenie z bazą danych
var db *sql.DB 
//sql.DB to duży obiekt (połączenie z bazą). Przekazując wskaźnik, nie kopiujemy całego obiektu.
//ta zmienna żyje poza funkcjami (zmienna globalna), każda funkcja może jej użyć
//Funkcja messageHandler (callback MQTT) nie może przyjmować dodatkowych argumentów, więc db musi być dostępne globalnie.



// Funkcja zapisująca odczyt temperatury
func saveTemperature(nodeID string, value float64, timestamp time.Time) { //typy po nazwie argumentu
    //Nazwa saveTemperature (mała litera = prywatna, tylko w tym pakiecie)
    //db.Exec to metoda na obiekcie db (np insert)
    //timestamp, nodeID i value to wartości dla $1,$2 i $3
    //_, err := db.Exec(...) - '_'-ignoruje pierwszą zwracaną wartość (sql.result), err to zmienna na błąd
    //:= ponieważ err jest nową zmienną (inaczej =)
    _, err := db.Exec(` 
        INSERT INTO sensor_readings (time, node_id, temperature)
        VALUES ($1, $2, $3)`,
        timestamp, nodeID, value)
    if err != nil {
        log.Printf("Błąd zapisu temperatury: %v", err)
    } else {
        log.Printf("Zapisano temp: node=%s, value=%.2f", nodeID, value)
    }
    //Zasada w Go: Sprawdzanie błędów jest jawne. Nie ma wyjątków (try/catch). 
    //Każda funkcja która może zawieść, zwraca error
}

// Funkcja zapisująca PM2.5 (patrz funkcja wyżej)
func savePM25(nodeID string, value int, timestamp time.Time) {
    _, err := db.Exec(`
        INSERT INTO sensor_readings (time, node_id, pm25)
        VALUES ($1, $2, $3)`,
        timestamp, nodeID, value)
    if err != nil {
        log.Printf("Błąd zapisu PM2.5: %v", err)
    } else {
        log.Printf("Zapisano PM2.5: node=%s, value=%d", nodeID, value)
    }
}

// Funkcja zapisująca alert
func saveAlert(nodeID string, alertType string, value interface{}, timestamp time.Time) {
    //value interface{} - Pusty interfejs - może przechować DOWOLNY typ (jak void* w C, ale bezpieczny)
    //Co to interface{}? W Go każdy typ implementuje pusty interfejs. To jak any w TypeScript lub Object w Java.
    // Dla uproszczenia: zapisujemy jako JSON w polu alert_type
    // W docelowej tabeli trzeba dodać kolumnę alert_data (JSONB)
    _, err := db.Exec(`
        INSERT INTO sensor_readings (time, node_id, alert_type)
        VALUES ($1, $2, $3)`,
        timestamp, nodeID, alertType)
    if err != nil {
        log.Printf("Błąd zapisu alertu: %v", err)
    } else {
        log.Printf("Zapisano alert: node=%s, type=%s", nodeID, alertType)
    }
}
//To jest callback - funkcja wywoływana za każdym razem gdy przychodzi wiadomość MQTT.
// Callback dla wiadomości MQTT
func messageHandler(client mqtt.Client, msg mqtt.Message) {
    //client mqtt.Client - Połączenie MQTT (możesz wysłać odpowiedź)
    //msg mqtt.Message - Odebrana wiadomość
    topic := msg.Topic() //Pobiera temat MQTT (np. "sentinel/node/01/temperature")
    payload := string(msg.Payload()) //Pobiera treść ([]byte) i zamienia na string
    timestamp := time.Now() //Aktualny czas (typ time.Time), w tych tu deklaracjach go sam wnioskuje typ

    log.Printf("Odebrano MQTT: %s -> %s", topic, payload)

    // Parsowanie topicu: sentinel/node/01/temperature
    parts := strings.Split(topic, "/") //Dzieli string na tablicę po separatorze "/"
    if len(parts) < 4 {
        log.Printf("Nieznany format topicu: %s", topic)
        return
    }

    nodeID := parts[2]       // "01"
    metric := parts[3]       // "temperature", "pm25", "accel", "alerts"

    //Nie trzeba pisać break - Go nie "wpada" w kolejne case (jak w C)
    //Możesz używać dowolnych typów (string, int, float)
    //default - gdy żaden case nie pasuje

    switch metric {
    case "temperature":
        var temp float64
        fmt.Sscanf(payload, "%f", &temp) //Jak sscanf w C: parsuje string na float. 
        //&temp to wskaźnik (adres) gdzie zapisać wynik
        //Dlaczego &temp? Go przekazuje argumenty przez wartość (kopiuje). 
        //Aby funkcja mogła zmienić zmienną w miejscu wywołania, potrzebuje wskaźnika.
        saveTemperature(nodeID, temp, timestamp)

    case "pm25":
        var pm25 int
        fmt.Sscanf(payload, "%d", &pm25)
        savePM25(nodeID, pm25, timestamp)

    case "accel":
        // Format: "X=-10000,Y=0,Z=16384" (na razie pomijamy, można rozszerzyć)
        log.Printf("Akcelerometr (pomijamy zapis): %s", payload)

    case "alerts":
        var alert AlertMessage
        //Parsuje JSON ze stringa do struktury. []byte(payload) konwertuje string na slice bajtów
        if err := json.Unmarshal([]byte(payload), &alert); err == nil {
            saveAlert(nodeID, alert.Type, alert.Value, timestamp)
        } else {
            log.Printf("Błąd parsowania alertu: %v", err)
        }

    default:
        log.Printf("Nieznana metryka: %s", metric)
    }
}
//Zasada: main() jest punktem startowym programu. Tylko w pakiecie main.
func main() {
    log.Println("=== Go Ingest Service ===")

    // --- POŁĄCZENIE Z BAZĄ DANYCH ---
    // String formatowany
    connStr := fmt.Sprintf("host=%s port=%d user=%s password=%s dbname=%s sslmode=disable",
        DB_HOST, DB_PORT, DB_USER, DB_PASSWORD, DB_NAME)
    
    //Dlaczego var err error? Bo później użyjemy = a nie :=.
    //:= stworzyłoby NOWĄ zmienną db (lokalną), a nie przypisało do globalnej.
    var err error
    db, err = sql.Open("postgres", connStr) //- otwiera połączenie (ale NIE ŁĄCZY! tylko tworzy obiekt).
    //Łączy się dopiero przy pierwszym zapytaniu.
    if err != nil {
        log.Fatal("Błąd otwarcia bazy:", err)
    }
    defer db.Close() // Odkłada wykonanie funkcji do momentu wyjścia z main() (nawet przez log.Fatal).

    err = db.Ping()
    if err != nil {
        log.Fatal("Błąd połączenia z bazą:", err)
    }
    log.Println("Połączono z TimescaleDB")
    //db.Ping() - faktycznie łączy się z bazą i sprawdza czy odpowiada. sql.Open() tego nie robi.

    // --- POŁĄCZENIE Z MQTT ---
    opts := mqtt.NewClientOptions() //Tworzy strukturę z opcjami (domyślne wartości) 
    opts.AddBroker(MQTT_BROKER) //Dodaje adres brokera
    opts.SetClientID(MQTT_CLIENTID) //Ustawia unikalne ID
    opts.SetDefaultPublishHandler(messageHandler) //Ustawia callback na naszą funkcję!!!

    client := mqtt.NewClient(opts) //Tworzy klienta MQTT (nie łączy jeszcze)
    if token := client.Connect(); token.Wait() && token.Error() != nil { //To jest IDIOMATYCZNE dla MQTT w Go:
        log.Fatal("Błąd MQTT:", token.Error())
    }
    log.Println("Połączono z MQTT brokerem")
    //token := client.Connect()	Rozpoczyna łączenie, zwraca token
    //token.Wait()	Blokuje aż operacja się zakończy (jak wait na Future)
    //token.Error() != nil	Sprawdza czy był błąd

    // --- SUBSTRYPCJA ---
    //[]string - slice (dynamiczna tablica) stringów
    //+ w topicu MQTT - wildcard (jeden poziom). sentinel/node/+/temperature pasuje do wszystkiego w miejscu plusa
    topics := []string{
        "sentinel/node/+/temperature",
        "sentinel/node/+/pm25",
        "sentinel/node/+/accel",
        "sentinel/alerts",
    }

    for _, topic := range topics { //range topics	Iteruje przez slice, zwraca (indeks, wartość)
        //_	Ignoruje indeks (więc mamy tylko wartość)
        //dla każdego tematu subskrybuje
        token := client.Subscribe(topic, 1, nil)
        token.Wait()
        if token.Error() != nil {
            log.Printf("Błąd subskrypcji %s: %v", topic, token.Error())
        } else {
            log.Printf("Subskrybuję: %s", topic)
        }
    }

    log.Println("Ingest service uruchomiony. Ctrl+C aby zakończyć.")
    
    // Blokuj główny wątek (czekaj w nieskończoność)
    select {} //Główny wątek nie może się zakończyć (bo wtedy program kończy działanie). Potrzebujemy czegoś co blokuje w nieskończoność.
}
//1. Go ładuje pakiet main
//2. Inicjalizuje zmienne globalne (db = nil)
//3. Wywołuje func main()
//4. main() wykonuje:
 //  a) Połączenie z TimescaleDB
 //  b) defer db.Close() (odkłada)
 //  c) Sprawdza połączenie (Ping)
 //  d) Konfiguruje MQTT
 //  e) Łączy z MQTT
 //  f) Subskrybuje 4 tematy
 //  g) Wypisuje "uruchomiony"
 //  h) select {} - BLOKUJE (program działa)
//5. Gdy przychodzi wiadomość MQTT:
 //  a) Biblioteka MQTT wywołuje messageHandler() w osobnym goroutine (lekkim wątku)
 //  b) messageHandler parsuje temat
 //  c) Wywołuje saveTemperature/savePM25/saveAlert
 //  d) Funkcje zapisują do bazy
 //  e) Wracają (callback kończy)
//6. Program działa aż do Ctrl+C