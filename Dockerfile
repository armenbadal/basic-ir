FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Հիմնական գործիքներ
RUN apt-get update && apt-get install -y \
    software-properties-common \
    wget \
    curl \
    gnupg \
    lsb-release \
    git \
    build-essential \
    ninja-build \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# CMake 4.2.6 (պաշտոնական binary release, Kitware/CMake GitHub-ից)
# apt-ի cmake package-ը սովորաբար շատ ավելի հին version է, դրա համար manual install
RUN wget -q https://github.com/Kitware/CMake/releases/download/v4.2.6/cmake-4.2.6-linux-x86_64.sh \
        -O /tmp/cmake-install.sh && \
    chmod +x /tmp/cmake-install.sh && \
    /tmp/cmake-install.sh --skip-license --prefix=/usr/local && \
    rm /tmp/cmake-install.sh

# LLVM 20 տեղադրում (պաշտոնական apt.llvm.org սկրիպտով)
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 20 all && \
    rm llvm.sh

# GCC 16 (վերջին stable version) — ubuntu-toolchain-r/test PPA-ի միջոցով,
# քանի որ Ubuntu 24.04-ի default repo-ում միայն GCC 13-ն է հասանելի
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    apt-get update && \
    apt-get install -y gcc-16 g++-16 && \
    rm -rf /var/lib/apt/lists/*

# gcc-16/g++-16-ը գրանցում ենք update-alternatives-ում (առանձին խմբում՝ "gcc"/"g++" անունով),
# բայց cc/c++ default-ը թողնում ենք clang-ի վրա (ինչպես ավելի վաղ կարգավորել ենք)
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-16 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-16 100

# clang/clang++-ը դարձնում ենք default (update-alternatives)
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-20 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-20 100 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/clang-20 100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-20 100

# Catch2 v3-ը build անում ենք source-ից (apt-ի տարբերակը սովորաբար հին է)
RUN git clone --depth 1 --branch v3.7.1 https://github.com/catchorg/Catch2.git /tmp/Catch2 && \
    cmake -S /tmp/Catch2 -B /tmp/Catch2/build \
        -DBUILD_TESTING=OFF \
        -DCMAKE_BUILD_TYPE=Release \
        -GNinja && \
    cmake --build /tmp/Catch2/build --target install && \
    rm -rf /tmp/Catch2

WORKDIR /workspace

CMD ["/bin/bash"]
