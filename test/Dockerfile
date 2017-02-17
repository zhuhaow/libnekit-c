FROM rikorose/gcc-cmake:gcc-5

RUN apt-get update && apt-get install -y --no-install-recommends valgrind \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*