CC      = gcc
CFLAGS  = -Wall -Wextra -g
LDFLAGS = -lpaho-mqtt3c

# Wszystkie źródła
SRCS    = main_all.c \
          bme680/bme680.c \
          pms5003/pms5003.c \
          mpu6050/mpu6050.c \
          mqtt_client/mqtt_client.c

TARGET  = sentinel_gateway

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)