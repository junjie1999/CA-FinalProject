# CA-FP Group Project

This repository contains the code and instructions to run the CA-FP (Computer Architecture Final Project) Dockerized environment. Use Docker to pull the pre-built image, explore the baseline CPU implementation, and develop your own extensions—everything stays in sync between your host and the container.

---

## :tools: Prerequisites

- [Docker](https://docs.docker.com/get-docker/) installed on your machine  
- (Optional) `git` if you plan to version‑control your changes

---

## :rocket: Quick Start

1. **Pull the Docker image**  
   ```bash
   docker pull amansinhaatnycu/ca-fp:v2
   ```

2. **Run the container**
   ```bash
   docker run -it --rm \
   -v "$(pwd):/home/CA-FP1" \
   amansinhaatnycu/ca-fp:v2 \
   bash
   ```
3. **Run the program**
   ```bash
   cd /home/CA-FP1/baseline/; make run
   ```
