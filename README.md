# CA-FP Group Project

This repository contains the code and instructions to run the CA-FP (Computer Architecture Final Project) dockerized environment.


## Prerequisites

- [Docker](https://docs.docker.com/get-docker/) installed on your machine  
- (Optional) `git` if you plan to version‑control your changes


## Setup

### 1. **Pull the Docker image**  
   ```bash
   docker pull amansinhaatnycu/ca-fp:v2
   ```

### 2. **Run the container in background**
   ```bash
   docker run -it --rm \
   -v "${WORK_DIR}:/home/CA-FP1" \
   amansinhaatnycu/ca-fp:v2 \
   bash
   ```
   Command `-v "${WORK_DIR}:/home/CA-FP1"` enables you to modify code outside the docker container.

   Variable `WORK_DIR` should be an absolute path on your local computer.

### 3. **Connect to the running container**
   ```bash
   docker exec -w /home/CA-FP1 -it $CONTAINER_ID bash
   ```
   Where variable `CONTAINER_ID` is the ID of the running container.

   You can get the ID by running command `docker ps -a`.

