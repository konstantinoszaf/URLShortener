APP_NAME         := URLShortener
DOCKER_NAME      := url_shortener
DOCKER_DIR       := ops/docker
LOCAL_BUILD_DIR  := build
DOCKER_PORT      := 8080
VCPKG_TOOLCHAIN  := $(HOME)/vcpkg/scripts/buildsystems/vcpkg.cmake

.PHONY: all local clean test test-run
all: build-image

local:
	@mkdir -p $(LOCAL_BUILD_DIR)
	cmake \
	  -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) \
	  -DCMAKE_BUILD_TYPE=Release \
	  -DBUILD_TESTS=OFF \
	  -S . -B $(LOCAL_BUILD_DIR)

	cmake --build $(LOCAL_BUILD_DIR) --parallel

clean:
	cmake --build $(LOCAL_BUILD_DIR) --target clean
	rm -rf $(LOCAL_BUILD_DIR)

test:
	@mkdir -p $(LOCAL_BUILD_DIR)
	cmake \
	  -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) \
	  -DCMAKE_BUILD_TYPE=Debug \
	  -DBUILD_TESTS=ON \
	  -S . \
	  -B $(LOCAL_BUILD_DIR)
	@cmake --build $(LOCAL_BUILD_DIR) --parallel

test-run:
	@cd $(LOCAL_BUILD_DIR) && ctest --output-on-failure

build-image:
	docker build -f $(DOCKER_DIR)/Dockerfile -t $(DOCKER_NAME):latest .

deploy:
	docker rm -f $(DOCKER_NAME)
	docker run -d --name $(DOCKER_NAME) \
					-p $(DOCKER_PORT):$(DOCKER_PORT) \
					-e API_KEY_BITLY=${API_KEY_BITLY} \
					-e API_KEY_TINYURL=${API_KEY_TINYURL} \
					 $(DOCKER_NAME):latest