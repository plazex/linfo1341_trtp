BUILD_DIR = build
TEST_DIR = tests
SENDER_DIR = src/sender
RECEIVER_DIR = src/receiver

.PHONY: all

all: test sender receiver

test:
	$(MAKE) -C $(TEST_DIR)

sender:
	$(MAKE) -C $(SENDER_DIR)

receiver:
	$(MAKE) -C $(RECEIVER_DIR)

.PHONY: clean

clean:
	rm -r $(BUILD_DIR)