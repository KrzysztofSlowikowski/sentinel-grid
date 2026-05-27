CC      = gcc
CFLAGS  = -Wall -Wextra -g
LDFLAGS = -lpaho-mqtt3c

# Wspólne źródła (czujniki i MQTT)
COMMON_SRCS = bme680/bme680.c \
              pms5003/pms5003.c \
              mpu6050/mpu6050.c \
              mqtt_client/mqtt_client.c

# Node 1
NODE1_SRCS = main_all.c $(COMMON_SRCS)
NODE1_TARGET = sentinel_gateway

# Node 2
NODE2_SRCS = main_node2.c $(COMMON_SRCS)
NODE2_TARGET = sentinel_gateway2

# Kompiluj node 1
$(NODE1_TARGET): $(NODE1_SRCS)
	$(CC) $(CFLAGS) $(NODE1_SRCS) -o $(NODE1_TARGET) $(LDFLAGS)

# Kompiluj node 2
$(NODE2_TARGET): $(NODE2_SRCS)
	$(CC) $(CFLAGS) $(NODE2_SRCS) -o $(NODE2_TARGET) $(LDFLAGS)

# Kompiluj oba
all: $(NODE1_TARGET) $(NODE2_TARGET)

# Uruchom node 1
run: $(NODE1_TARGET)
	./$(NODE1_TARGET)

# Uruchom node 2
run2: $(NODE2_TARGET)
	./$(NODE2_TARGET)

# Uruchom oba (w tle)
run-both: $(NODE1_TARGET) $(NODE2_TARGET)
	./$(NODE1_TARGET) &
	./$(NODE2_TARGET) &

clean:
	rm -f $(NODE1_TARGET) $(NODE2_TARGET)